/**
 * @file NmeaParserStats.cpp
 * @brief NMEA 解析器统计查询方法实现
 *
 * 从 NmeaParser.cpp 拆分而来，包含统计 getter 和 reset 方法。
 */

#include "utils/gps/NmeaParser.h"

/** @brief 获取统计快照 @return 统计结构体副本 */
GpsParserStats NmeaParser::stats() const
{
    return m_stats;
}

/** @brief 重置统计计数器(不清除轨迹和当前位置) */
void NmeaParser::resetStatistics()
{
    m_stats = GpsParserStats();
}
