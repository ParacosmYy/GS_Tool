/**
 * @file NmeaParser.h
 * @brief NMEA 0183 协议解析器
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 增量式解析 NMEA 语句(GGA/RMC/GSV/GSA/VTG)，
 * 提取 GPS 位置、卫星信息、速度航向等数据。
 */

#ifndef NMEAPARSER_H
#define NMEAPARSER_H

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QSet>
#include <QVector>

#include "utils/gps/GpsTypes.h"

/**
 * @class NmeaParser
 * @brief NMEA 0183 增量解析器
 *
 * 调用 feedData() 注入串口原始字节流，自动检测语句边界、
 * 校验和验证，解析后发射 positionUpdated / satellitesUpdated 信号。
 */
class NmeaParser : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造 NMEA 解析器 @param parent 父对象 */
    explicit NmeaParser(QObject *parent = nullptr);

    /** @brief 喂入原始串口数据进行增量解析 @param data 串口接收的原始字节 */
    void feedData(const QByteArray &data);

    /** @brief 获取最新位置 @return 当前位置快照 */
    GpsPosition currentPosition() const;

    /** @brief 获取当前卫星列表 @return 卫星信息向量 */
    QVector<GpsSatellite> satellites() const;

    /** @brief 获取轨迹点列表 @return 累计的轨迹点 */
    QVector<GpsTrackPoint> trackPoints() const;

    /** @brief 获取解析统计 @return 统计快照 */
    GpsParserStats stats() const;

    /** @brief 重置解析器状态和统计计数器 */
    void reset();

    // ---- 统计计数内联访问 ----
    /** @brief 累计解析语句数 */
    quint64 totalSentencesParsed() const { return m_stats.sentencesParsed; }
    /** @brief 累计校验和错误数 */
    quint64 totalChecksumErrors() const { return m_stats.checksumErrors; }
    /** @brief 累计位置解码数 */
    quint64 totalPositionsDecoded() const { return m_stats.positionsDecoded; }
    /** @brief 累计见过的卫星数(去重) */
    quint64 totalSatellitesSeen() const { return m_stats.satellitesSeen; }
    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 位置更新信号 @param position 新位置 */
    void positionUpdated(const GpsPosition &position);

    /** @brief 卫星列表更新信号 @param satellites 当前可见卫星 */
    void satellitesUpdated(const QVector<GpsSatellite> &satellites);

    /** @brief NMEA 语句解析完成信号 @param sentence 完整语句内容 */
    void sentenceReceived(const QByteArray &sentence);

private:
    /** @brief 尝试从缓冲区提取并解析完整语句 @return 是否成功解析至少一条 */
    bool tryParseNext();

    /** @brief 校验和验证 @param sentence 含 $ 和 * 的语句 @return 校验和是否通过 */
    bool validateChecksum(const QByteArray &sentence) const;

    /** @brief 分发到具体语句解析器 @param fields 逗号分隔的字段列表 */
    void dispatchSentence(const QVector<QByteArray> &fields);

    /** @brief 解析 GGA 语句 @param fields 字段列表 */
    void parseGga(const QVector<QByteArray> &fields);

    /** @brief 解析 RMC 语句 @param fields 字段列表 */
    void parseRmc(const QVector<QByteArray> &fields);

    /** @brief 解析 GSV 语句 @param fields 字段列表 */
    void parseGsv(const QVector<QByteArray> &fields);

    /** @brief 解析 GSA 语句 @param fields 字段列表 */
    void parseGsa(const QVector<QByteArray> &fields);

    /** @brief 解析 VTG 语句 @param fields 字段列表 */
    void parseVtg(const QVector<QByteArray> &fields);

    /** @brief NMEA 坐标转十进制度 @param raw NMEA 格式坐标(dddmm.mmmm) @param hemisphere 半球标识(N/S/E/W) @return 十进制度 */
    static double nmeaToDecimal(const QByteArray &raw, char hemisphere);

    QByteArray         m_buffer;           ///< 未解析的字节缓冲区
    GpsPosition        m_currentPos;       ///< 最新位置
    QVector<GpsSatellite> m_satellites;    ///< 当前卫星列表
    QVector<GpsTrackPoint> m_trackPoints;  ///< 轨迹点
    QSet<int>          m_seenSatIds;       ///< 历史出现过的卫星PRN集合
    GpsParserStats     m_stats;            ///< 统计
};

#endif // NMEAPARSER_H
