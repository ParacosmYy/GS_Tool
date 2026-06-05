/**
 * @file GpsTypes.h
 * @brief GPS/NMEA 数据类型定义
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 定义 GPS 位置、卫星、定位类型、轨迹点及统计结构体。
 */

#ifndef GPSTYPES_H
#define GPSTYPES_H

#include <QDateTime>
#include <QVector>
#include <QtGlobal>

/**
 * @enum GpsFixType
 * @brief GPS 定位类型
 */
enum class GpsFixType {
    None,       ///< 无定位
    GPS,        ///< 标准 GPS
    DGPS,       ///< 差分 GPS
    PPS,        ///< PPS 定位
    RTK,        ///< RTK 固定解
    FloatRTK,   ///< RTK 浮点解
    Estimated,  ///< 估算定位
    Manual      ///< 手动输入
};

/**
 * @struct GpsPosition
 * @brief GPS 位置信息
 */
struct GpsPosition {
    double latitude   = 0.0;   ///< 纬度(十进制度, 正北负南)
    double longitude  = 0.0;   ///< 经度(十进制度, 正东负西)
    double altitude   = 0.0;   ///< 海拔高度(米)
    double speed      = 0.0;   ///< 地面速度(米/秒)
    double course     = 0.0;   ///< 地面航向(度, 0~359.99)
    double hdop       = 0.0;   ///< 水平精度因子
    double vdop       = 0.0;   ///< 垂直精度因子
    double pdop       = 0.0;   ///< 位置精度因子
    int    numSatellites = 0;   ///< 使用中的卫星数
    GpsFixType fixType = GpsFixType::None; ///< 定位类型
    QDateTime timestamp;       ///< UTC 时间戳
    bool    valid      = false;///< 数据是否有效
};

/**
 * @struct GpsSatellite
 * @brief 单颗卫星信息
 */
struct GpsSatellite {
    int  id        = 0;    ///< 卫星 PRN 编号
    int  elevation = 0;    ///< 仰角(度, 0~90)
    int  azimuth   = 0;    ///< 方位角(度, 0~359)
    int  snr       = 0;    ///< 信噪比(dB-Hz, 0~99)
    bool inUse     = false;///< 是否参与定位解算
};

/**
 * @struct GpsTrackPoint
 * @brief 轨迹点(用于轨迹可视化)
 */
struct GpsTrackPoint {
    GpsPosition pos;       ///< 位置信息
    QDateTime   timestamp; ///< 时间戳
    double      speed  = 0.0; ///< 速度(米/秒)
    double      course = 0.0; ///< 航向(度)
};

/**
 * @struct GpsParserStats
 * @brief NMEA 解析器统计
 */
struct GpsParserStats {
    quint64 sentencesParsed   = 0; ///< 成功解析的 NMEA 语句总数
    quint64 sentencesWithFix  = 0; ///< 包含有效定位的语句数
    quint64 checksumErrors    = 0; ///< 校验和失败数
    quint64 parseErrors       = 0; ///< 解析格式错误数
    quint64 bytesFed          = 0; ///< 累计输入字节数
    quint64 positionsDecoded  = 0; ///< 解码位置更新总数
    quint64 satellitesSeen    = 0; ///< 累计见过的卫星数(去重)
    quint64 ggaCount          = 0; ///< GGA 语句计数
    quint64 rmcCount          = 0; ///< RMC 语句计数
    quint64 gsvCount          = 0; ///< GSV 语句计数
    quint64 gsaCount          = 0; ///< GSA 语句计数
    quint64 vtgCount          = 0; ///< VTG 语句计数
};

/**
 * @struct GpsTrackWidgetStats
 * @brief 轨迹控件统计
 */
struct GpsTrackWidgetStats {
    quint64 trackPointsDrawn  = 0; ///< 累计绘制的轨迹点数
    quint64 redraws           = 0; ///< paintEvent 触发次数
    quint64 autoFits          = 0; ///< 自动适配视口次数
    quint64 zoomEvents        = 0; ///< 缩放事件次数
    quint64 panEvents         = 0; ///< 平移事件次数
    quint64 waypointMarkers   = 0; ///< 航路点标记数
    quint64 gridRenders       = 0; ///< 网格绘制次数
};

#endif // GPSTYPES_H
