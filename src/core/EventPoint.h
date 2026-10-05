#pragma once

#include <QTime>
#include <QString>

class EventPoint
{
public:
    EventPoint(const unsigned& id, const QTime& time, const QString& name, const int& advanceTime = 60, const unsigned& flashTimes = 4)
        : m_id(id), m_time(time), m_name(name), m_advanceTime(advanceTime), m_flashTimes(flashTimes) {}

    const unsigned& id() const { return m_id; }
    const QTime& time() const { return m_time; }
    const QString& name() const { return m_name; }
    const int& advanceTime() const { return m_advanceTime; }
    const unsigned& flashTimes() const { return m_flashTimes; }
    bool isShowing() const { return m_isShowing; }
    void setShowing(bool isShowing) { m_isShowing = isShowing; }

private:
    unsigned m_id;
    QTime m_time;
    QString m_name;
    int m_advanceTime;
    unsigned m_flashTimes;
    bool m_isShowing = false;
};