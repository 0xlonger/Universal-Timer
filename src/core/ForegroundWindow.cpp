#include "core/ForegroundWindow.h"

#include <QFileInfo>

#if defined(Q_OS_WIN)

#include <windows.h>

static bool containsRect(const RECT& outer, const RECT& inner)
{
    return outer.left <= inner.left && outer.top <= inner.top && outer.right >= inner.right && outer.bottom >= inner.bottom;
}

ForegroundWindowInfo queryForegroundWindow()
{
    ForegroundWindowInfo info;
    HWND hwnd = GetForegroundWindow();
    if (!hwnd)
        return info;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0)
        return info;

    info.valid = true;
    info.own = pid == GetCurrentProcessId();
    wchar_t buffer[1024];
    int length = GetWindowTextW(hwnd, buffer, 1024);
    info.title = QString::fromWCharArray(buffer, qMax(0, length));
    length = GetClassNameW(hwnd, buffer, 1024);
    info.className = QString::fromWCharArray(buffer, qMax(0, length));

    if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        DWORD size = 1024;
        if (QueryFullProcessImageNameW(process, 0, buffer, &size))
            info.processName = QFileInfo(QString::fromWCharArray(buffer, int(size))).completeBaseName(); // 和 ClassIsland 一样不带 .exe
        CloseHandle(process);
    }

    info.minimized = IsIconic(hwnd);
    // 与 ClassIsland 相同：桌面（WorkerW / Progman）不算最大化或全屏；窗口盖住整个屏幕算全屏，盖住工作区（不含任务栏）算最大化
    if (info.className != "WorkerW" && info.className != "Progman") {
        RECT rect;
        MONITORINFO monitor = {};
        monitor.cbSize = sizeof(monitor);
        const POINT origin = { 0, 0 };
        if (GetWindowRect(hwnd, &rect) && GetMonitorInfoW(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &monitor)) {
            info.fullscreen = containsRect(rect, monitor.rcMonitor);
            info.maximized = containsRect(rect, monitor.rcWork);
        }
    }
    return info;
}

#elif defined(UT_HAVE_XCB)

#include <QFile>
#include <QHash>

#include <xcb/xcb.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

static xcb_connection_t* x11Connection()
{
    static xcb_connection_t* connection = nullptr;
    static bool tried = false;
    if (!tried) {
        tried = true;
        connection = xcb_connect(nullptr, nullptr);
        if (xcb_connection_has_error(connection)) {
            xcb_disconnect(connection);
            connection = nullptr;
        }
    }
    return connection;
}

static xcb_atom_t x11Atom(xcb_connection_t* connection, const char* name)
{
    static QHash<QByteArray, xcb_atom_t> atoms;
    const auto it = atoms.constFind(name);
    if (it != atoms.constEnd())
        return *it;
    xcb_atom_t atom = XCB_ATOM_NONE;
    if (xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, xcb_intern_atom(connection, 0, uint16_t(strlen(name)), name), nullptr)) {
        atom = reply->atom;
        free(reply);
    }
    atoms.insert(name, atom);
    return atom;
}

static QByteArray x11Property(xcb_connection_t* connection, xcb_window_t window, xcb_atom_t property, xcb_atom_t type)
{
    QByteArray result;
    xcb_get_property_reply_t* reply = xcb_get_property_reply(connection, xcb_get_property(connection, 0, window, property, type, 0, 4096), nullptr);
    if (reply) {
        if (reply->type != XCB_ATOM_NONE)
            result = QByteArray(static_cast<const char*>(xcb_get_property_value(reply)), xcb_get_property_value_length(reply)); // 已经是字节数
        free(reply);
    }
    return result;
}

static quint32 x11Cardinal(const QByteArray& value)
{
    quint32 result = 0;
    if (value.size() >= 4)
        memcpy(&result, value.constData(), 4);
    return result;
}

ForegroundWindowInfo queryForegroundWindow()
{
    ForegroundWindowInfo info;
    xcb_connection_t* connection = x11Connection();
    if (!connection)
        return info;
    const xcb_window_t root = xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;

    const xcb_window_t window = x11Cardinal(x11Property(connection, root, x11Atom(connection, "_NET_ACTIVE_WINDOW"), XCB_ATOM_WINDOW));
    if (window == XCB_WINDOW_NONE)
        return info;
    const quint32 pid = x11Cardinal(x11Property(connection, window, x11Atom(connection, "_NET_WM_PID"), XCB_ATOM_CARDINAL));
    info.valid = true;
    info.own = pid == quint32(getpid());
    QByteArray title = x11Property(connection, window, x11Atom(connection, "_NET_WM_NAME"), x11Atom(connection, "UTF8_STRING"));
    info.title = title.isEmpty() ? QString::fromLatin1(x11Property(connection, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING)) : QString::fromUtf8(title);
    const QList<QByteArray> wmClass = x11Property(connection, window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING).split('\0'); // “实例名\0类名\0”
    info.className = wmClass.size() >= 2 ? QString::fromLocal8Bit(wmClass[1]) : QString();
    if (pid != 0) {
        QFile comm(QString("/proc/%1/comm").arg(pid));
        if (comm.open(QIODevice::ReadOnly))
            info.processName = QString::fromLocal8Bit(comm.readAll()).trimmed();
    }

    const QByteArray states = x11Property(connection, window, x11Atom(connection, "_NET_WM_STATE"), XCB_ATOM_ATOM);
    bool maximizedVert = false, maximizedHorz = false;
    for (int i = 0; i + 4 <= states.size(); i += 4) {
        const xcb_atom_t state = x11Cardinal(states.mid(i, 4));
        if (state == x11Atom(connection, "_NET_WM_STATE_FULLSCREEN")) info.fullscreen = true;
        else if (state == x11Atom(connection, "_NET_WM_STATE_MAXIMIZED_VERT")) maximizedVert = true;
        else if (state == x11Atom(connection, "_NET_WM_STATE_MAXIMIZED_HORZ")) maximizedHorz = true;
        else if (state == x11Atom(connection, "_NET_WM_STATE_HIDDEN")) info.minimized = true;
    }
    info.maximized = maximizedVert && maximizedHorz;
    return info;
}

#else

ForegroundWindowInfo queryForegroundWindow()
{
    return ForegroundWindowInfo();
}

#endif
