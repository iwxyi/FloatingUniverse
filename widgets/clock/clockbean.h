#ifndef CLOCKBEAN_H
#define CLOCKBEAN_H

#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include "myjson.h"

enum ClockType
{

};

/**
 * @brief 闹钟种类
 */
enum AlarmType
{
    AlarmNone = 0,
    AlarmStar = 1, // 星级闹钟：面板内摇晃，作为提醒
    AlarmMoon = 2, // 月级闹钟：出现面板并摇晃
    AlarmSuns = 3, // 日级闹钟：全屏出现动画
};

/**
 * @brief 时间单位
 */
enum TimeUnit
{
    TimeNone = 0,
    TimeSecond,
    TimeMinute,
    TimeHour,
    TimeDay,
    TimeMonth,
    TimeYear
};

static QStringList timeUnitString {
    "msecond", "second", "minute", "hour", "day", "month", "year"
};

class ClockBean : public QObject
{
    Q_OBJECT
public:
    ClockBean(QObject* parent = nullptr) : QObject(parent)
    {}

    // 时间
    int fixedTime[TimeYear+1] = {}; // 指定 年月日时分秒x   <-
    int weeks = 0; // 至少8位，7654321x   <-

    // 周期
    TimeUnit intervalUnit = TimeNone;
    int intervalCount = 0;

    // 其他参数
    bool enabled = false; // 是否启用
    QString note; // 备注
    AlarmType alarm = AlarmNone; // 提醒方式
    bool singleShot = true; // 单次触发

    // 非用户修改参数
    bool overdue = false; // 是否过期
    QTimer* timer = nullptr; // 定时器
    QDateTime alarmTime; // 下一次闹钟时间

    static ClockBean* fromJson(MyJson json, QObject* parent = nullptr)
    {
        ClockBean* cb = new ClockBean(parent);
        for (int i = 1; i <= TimeYear; i++)
        {
            if (json.contains(timeUnitString.at(i)))
                cb->fixedTime[i] = json.i(timeUnitString.at(i));
        }
        cb->weeks = json.i("weeks");
        cb->note = json.s("note");
        cb->intervalUnit = TimeUnit(json.i("intervalUnit"));
        cb->intervalCount = json.i("intervalCount");
        cb->alarm = AlarmType(json.i("alarm"));
        cb->enabled = json.b("enabled");
        return cb;
    }

    MyJson toJson()
    {
        MyJson json;
        for (int i = 1; i <= TimeYear; i++)
        {
            if (fixedTime[i])
                json.insert(timeUnitString.at(i), fixedTime[i]);
        }
        if (weeks)
            json.insert("weeks", weeks);
        if (!note.isEmpty())
            json.insert("note", note);
        if (intervalCount)
        {
            json.insert("intervalCount", intervalCount);
            json.insert("intervalUnit", int(intervalUnit));
        }
        json.insert("alarm", int(alarm));
        json.insert("enabled", enabled);
        return json;
    }

    /**
     * @brief 根据时间配置，开启定时器
     */
    bool startTimer()
    {
        if (!timer)
        {
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [=]{
                emit timeout();

                if (singleShot)
                {
                    overdue = true;
                }
                else
                {
                    fixedTime[TimeYear] = alarmTime.date().year();
                    fixedTime[TimeMonth] = alarmTime.date().month();
                    fixedTime[TimeDay] = alarmTime.date().day();
                }
                emit autoModified();
            });
        }

        // 当前信息
        QDate today = QDate::currentDate();
        QTime totime = QTime::currentTime();
        QDateTime currentDT = QDateTime::currentDateTime();

        // 时间（必定都有，也可能是0）
        int hour = fixedTime[TimeHour];
        int minute = fixedTime[TimeMinute];
        QTime time(hour, minute);
        bool nextDay = time >= totime; // 今天时间到了，需要到下一天

        // 日期（至少有一个）
        QDate date;
        int day = fixedTime[TimeDay];
        if (day) // 指定日
        {
            int month = fixedTime[TimeMonth];
            int year = fixedTime[TimeYear];
            if (month == 0)
                month = today.month();
            if (year == 0)
                year = today.year();
            date.setDate(year, month, day);
        }
        else if (weeks) // 指定周几
        {
            date = QDate::currentDate();
            int wday = QDate::currentDate().dayOfWeek();
            if (nextDay)
            {
                date = date.addDays(1);
                wday++;
            }

            // 查找最近的周几
            int _count = 7;
            while (_count--)
            {
                if (wday > 7)
                    wday = 1;
                if (weeks & (1 << wday))
                    break;
                wday++;
                date = date.addDays(1);
            }
        }
        else
        {
            // 未知的时间
            return false;
        }

        alarmTime = QDateTime(date, time);
        int interval = int(alarmTime.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch());
        if (interval <= 0)
        {
            if (singleShot) // 单次触发
            {
                overdue = true; // 记作已经过期了
                qInfo() << "提醒已到期";
                return false;
            }

            // 查找周期时间
            if (intervalUnit == TimeDay)
            {
                while (alarmTime.addDays(1) < currentDT);
            }
            else if (intervalUnit == TimeMonth)
            {
                while (alarmTime.addMonths(1) < currentDT);
            }
            else if (intervalUnit == TimeYear)
            {
                while (alarmTime.addMonths(1) < currentDT);
            }
            else
            {
                qWarning() << "未设置循环时间";
                return false;
            }
        }

        timer->setInterval(interval * 1000);
        timer->start();
        return true;
    }

    void stopTimer()
    {
        if (!timer)
            return ;

        timer->stop();
    }

signals:
    void timeout();
    void autoModified();
};

#endif // CLOCKBEAN_H
