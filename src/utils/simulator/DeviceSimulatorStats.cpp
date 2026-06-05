/**
 * @file DeviceSimulatorStats.cpp
 * @brief DeviceSimulator 统计辅助 -- 统计数据 JSON 序列化与格式化输出
 */
#include "utils/simulator/DeviceSimulator.h"

#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief 将运行统计数据转换为 JSON 对象
 * @param st 统计数据引用
 * @return QJsonObject 包含所有统计字段
 */
QJsonObject deviceSimStatsToJson(const SimulatorRunStats& st)
{
    QJsonObject obj;
    obj["commandsReceived"]   = static_cast<qint64>(st.commandsReceived);
    obj["responsesSent"]      = static_cast<qint64>(st.responsesSent);
    obj["commandsUnmatched"]  = static_cast<qint64>(st.commandsUnmatched);
    obj["totalBytesSent"]     = static_cast<qint64>(st.totalBytesSent);
    obj["totalBytesReceived"] = static_cast<qint64>(st.totalBytesReceived);
    obj["avgResponseDelayMs"] = st.avgResponseDelayMs;
    obj["echoCount"]          = static_cast<qint64>(st.echoCount);
    obj["noiseCount"]         = static_cast<qint64>(st.noiseCount);
    return obj;
}
