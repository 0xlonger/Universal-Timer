#pragma once

#include <QTime>
#include <QString>

class EventPoint
{
public:
    EventPoint(const QTime& time, const QString& name, const int& advanceTime = 60)
        : m_time(time), m_name(name), m_advanceTime(advanceTime) {}

    const QTime& time() const { return m_time; }
    const QString& name() const { return m_name; }
    const int& advanceTime() const { return m_advanceTime; }
    
private:
    QTime m_time;
    QString m_name;
    int m_advanceTime;
};