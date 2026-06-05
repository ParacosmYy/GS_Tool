/**
 * @file DeviceSimulatorPanelStats.cpp
 * @brief DeviceSimulatorPanel 统计辅助 -- 面板统计信息格式化
 */
#include "utils/simulator/DeviceSimulatorPanel.h"
#include "utils/simulator/DeviceSimulator.h"

#include <QString>

/**
 * @brief 格式化模拟器统计数据为显示字符串
 * @param st 统计数据引用
 * @return 格式化后的多行统计文本
 */
QString formatSimulatorStats(const SimulatorRunStats& st)
{
    return DeviceSimulatorPanel::tr(
        "接收: %1  发送: %2  未匹配: %3  "
        "发送字节: %4  接收字节: %5  平均延迟: %6ms")
        .arg(st.commandsReceived)
        .arg(st.responsesSent)
        .arg(st.commandsUnmatched)
        .arg(st.totalBytesSent)
        .arg(st.totalBytesReceived)
        .arg(st.avgResponseDelayMs, 0, 'f', 1);
}
