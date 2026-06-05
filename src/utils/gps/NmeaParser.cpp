/**
 * @file NmeaParser.cpp
 * @brief NMEA 0183 协议解析器核心实现
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 包含语句边界检测、校验和验证、GGA/RMC/GSV/GSA/VTG 解析、
 * 坐标转换(NMEA ddmm.mmmm → 十进制度)。
 */

#include "utils/gps/NmeaParser.h"

#include <QDate>
#include <QTime>
#include <QTimeZone>
#include <QtMath>

// ---------------------------------------------------------------------------
// Construction / reset
// ---------------------------------------------------------------------------

/** @brief 构造解析器 @param parent 父对象 */
NmeaParser::NmeaParser(QObject *parent)
    : QObject(parent)
{
}

/** @brief 获取最新位置快照 @return 当前位置 */
GpsPosition NmeaParser::currentPosition() const
{
    return m_currentPos;
}

/** @brief 获取卫星列表 @return 当前卫星信息向量 */
QVector<GpsSatellite> NmeaParser::satellites() const
{
    return m_satellites;
}

/** @brief 获取轨迹点 @return 累计的轨迹点 */
QVector<GpsTrackPoint> NmeaParser::trackPoints() const
{
    return m_trackPoints;
}

/** @brief 重置解析器状态和统计计数器 */
void NmeaParser::reset()
{
    m_buffer.clear();
    m_currentPos = GpsPosition();
    m_satellites.clear();
    m_trackPoints.clear();
    m_seenSatIds.clear();
    m_stats = GpsParserStats();
}

// ---------------------------------------------------------------------------
// Data feeding
// ---------------------------------------------------------------------------

/** @brief 喂入串口原始数据 @param data 原始字节 */
void NmeaParser::feedData(const QByteArray &data)
{
    m_buffer.append(data);
    m_stats.bytesFed += static_cast<quint64>(data.size());
    while (tryParseNext()) {
        // keep parsing until buffer exhausted
    }
}

// ---------------------------------------------------------------------------
// Sentence extraction
// ---------------------------------------------------------------------------

/** @brief 从缓冲区提取下一条完整 NMEA 语句并解析 @return 是否成功解析 */
bool NmeaParser::tryParseNext()
{
    const int dollarIdx = m_buffer.indexOf('$');
    if (dollarIdx < 0) {
        m_buffer.clear();
        return false;
    }
    // Discard leading garbage
    if (dollarIdx > 0) {
        m_buffer.remove(0, dollarIdx);
    }
    const int crlfIdx = m_buffer.indexOf("\r\n");
    if (crlfIdx < 0) {
        return false; // incomplete sentence, wait for more data
    }

    QByteArray sentence = m_buffer.left(crlfIdx);
    m_buffer.remove(0, crlfIdx + 2);

    if (!validateChecksum(sentence)) {
        ++m_stats.checksumErrors;
        return true; // sentence consumed, continue
    }

    // Split fields: $TALKER,field1,field2,...*CS
    QVector<QByteArray> fields;
    int starIdx = sentence.indexOf('*');
    QByteArray payload = (starIdx > 0)
                             ? sentence.mid(1, starIdx - 1)
                             : sentence.mid(1);
    const auto parts = payload.split(',');
    for (const auto &p : parts) {
        fields.append(p);
    }

    ++m_stats.sentencesParsed;
    dispatchSentence(fields);
    emit sentenceReceived(sentence);
    return true;
}

// ---------------------------------------------------------------------------
// Checksum
// ---------------------------------------------------------------------------

/** @brief XOR 校验和验证 @param sentence 完整语句(含 $ 和 *) @return 是否通过 */
bool NmeaParser::validateChecksum(const QByteArray &sentence) const
{
    const int starIdx = sentence.indexOf('*');
    if (starIdx < 1) {
        return false;
    }
    quint8 calc = 0;
    for (int i = 1; i < starIdx; ++i) {
        calc ^= static_cast<quint8>(sentence[i]);
    }
    bool ok = false;
    const quint8 expected = static_cast<quint8>(
        sentence.mid(starIdx + 1).toUInt(&ok, 16));
    return ok && (calc == expected);
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------

/** @brief 按语句类型分发解析 @param fields 逗号分隔的字段列表 */
void NmeaParser::dispatchSentence(const QVector<QByteArray> &fields)
{
    if (fields.isEmpty()) {
        return;
    }
    const QByteArray &tag = fields[0];
    // tag format: 2-char talker + 3-char sentence type
    if (tag.size() < 3) {
        ++m_stats.parseErrors;
        return;
    }
    const QByteArray type = tag.right(3);

    if (type == "GGA") {
        ++m_stats.ggaCount;
        parseGga(fields);
    } else if (type == "RMC") {
        ++m_stats.rmcCount;
        parseRmc(fields);
    } else if (type == "GSV") {
        ++m_stats.gsvCount;
        parseGsv(fields);
    } else if (type == "GSA") {
        ++m_stats.gsaCount;
        parseGsa(fields);
    } else if (type == "VTG") {
        ++m_stats.vtgCount;
        parseVtg(fields);
    }
    // ignore unsupported sentence types silently
}

// ---------------------------------------------------------------------------
// GGA: Global Positioning System Fix Data
// ---------------------------------------------------------------------------

/** @brief 解析 GGA 语句 @param fields 字段列表 */
void NmeaParser::parseGga(const QVector<QByteArray> &fields)
{
    // $--GGA,time,lat,N/S,lon,E/W,quality,numSV,HDOP,alt,M,sep,M,diffAge,diffStation*CS
    if (fields.size() < 10) {
        ++m_stats.parseErrors;
        return;
    }

    // UTC time: hhmmss.ss
    const QTime time = QTime::fromString(QString::fromLatin1(fields[1]),
                                          QStringLiteral("hhmmss.z"));
    if (time.isValid()) {
        m_currentPos.timestamp.setTime(time);
    }

    // Latitude
    if (!fields[2].isEmpty()) {
        m_currentPos.latitude = nmeaToDecimal(fields[2],
                                               fields[3].isEmpty() ? 'N' : fields[3][0]);
    }
    // Longitude
    if (!fields[4].isEmpty()) {
        m_currentPos.longitude = nmeaToDecimal(fields[4],
                                                fields[5].isEmpty() ? 'E' : fields[5][0]);
    }

    // Fix quality
    const int quality = fields[6].toInt();
    switch (quality) {
    case 0:  m_currentPos.fixType = GpsFixType::None;     break;
    case 1:  m_currentPos.fixType = GpsFixType::GPS;      break;
    case 2:  m_currentPos.fixType = GpsFixType::DGPS;     break;
    case 3:  m_currentPos.fixType = GpsFixType::PPS;      break;
    case 4:  m_currentPos.fixType = GpsFixType::RTK;      break;
    case 5:  m_currentPos.fixType = GpsFixType::FloatRTK; break;
    case 6:  m_currentPos.fixType = GpsFixType::Estimated; break;
    case 7:  m_currentPos.fixType = GpsFixType::Manual;   break;
    default: m_currentPos.fixType = GpsFixType::None;     break;
    }

    m_currentPos.numSatellites = fields[7].toInt();
    m_currentPos.hdop          = fields[8].toDouble();
    m_currentPos.altitude      = fields[9].toDouble();
    m_currentPos.valid = (quality > 0);

    if (m_currentPos.valid) {
        ++m_stats.sentencesWithFix;
        ++m_stats.positionsDecoded;
        emit positionUpdated(m_currentPos);
    }
}

// ---------------------------------------------------------------------------
// RMC: Recommended Minimum Navigation
// ---------------------------------------------------------------------------

/** @brief 解析 RMC 语句 @param fields 字段列表 */
void NmeaParser::parseRmc(const QVector<QByteArray> &fields)
{
    // $--RMC,time,status,lat,N/S,lon,E/W,speed,course,date,magVar,magVarDir,posMode*CS
    if (fields.size() < 10) {
        ++m_stats.parseErrors;
        return;
    }

    // Status: A=valid, V=warning
    const bool valid = (fields[2].size() == 1 && fields[2][0] == 'A');

    // UTC time
    const QTime time = QTime::fromString(QString::fromLatin1(fields[1]),
                                          QStringLiteral("hhmmss.z"));
    // UTC date: ddmmyy
    const QDate date = QDate::fromString(QString::fromLatin1(fields[9]),
                                          QStringLiteral("ddMMyy"));
    if (time.isValid() && date.isValid()) {
        QDateTime dt(date, time, QTimeZone::UTC);
        m_currentPos.timestamp = dt;
    }

    // Coordinates
    if (!fields[3].isEmpty()) {
        m_currentPos.latitude = nmeaToDecimal(fields[3],
                                               fields[4].isEmpty() ? 'N' : fields[4][0]);
    }
    if (!fields[5].isEmpty()) {
        m_currentPos.longitude = nmeaToDecimal(fields[5],
                                                fields[6].isEmpty() ? 'E' : fields[6][0]);
    }

    // Speed: knots → m/s
    m_currentPos.speed = fields[7].toDouble() * 0.514444;
    // Course: degrees true
    m_currentPos.course = fields[8].toDouble();

    m_currentPos.valid = valid;
    if (valid) {
        ++m_stats.sentencesWithFix;
        ++m_stats.positionsDecoded;

        // Append track point
        GpsTrackPoint tp;
        tp.pos       = m_currentPos;
        tp.timestamp = m_currentPos.timestamp;
        tp.speed     = m_currentPos.speed;
        tp.course    = m_currentPos.course;
        m_trackPoints.append(tp);

        emit positionUpdated(m_currentPos);
    }
}

// ---------------------------------------------------------------------------
// GSV: Satellites in View
// ---------------------------------------------------------------------------

/** @brief 解析 GSV 语句 @param fields 字段列表 */
void NmeaParser::parseGsv(const QVector<QByteArray> &fields)
{
    // $--GSV,totalMsg,msgNum,numSats,pr1,el1,az1,snr1,pr2,...*CS
    if (fields.size() < 4) {
        ++m_stats.parseErrors;
        return;
    }
    const int msgNum = fields[2].toInt(); // current message number (1-based)

    // On first message of a GSV sequence, clear old GSV data
    if (msgNum == 1) {
        // Mark existing satellites as not-yet-refreshed
        for (auto &sat : m_satellites) {
            sat.snr   = 0;
            sat.inUse = false;
        }
    }

    // Each GSV carries up to 4 satellites
    const int numBlocks = (fields.size() - 4) / 4;
    for (int i = 0; i < numBlocks; ++i) {
        const int base = 4 + i * 4;
        GpsSatellite sat;
        sat.id        = fields[base + 0].toInt();
        sat.elevation = fields[base + 1].toInt();
        sat.azimuth   = fields[base + 2].toInt();
        sat.snr       = fields[base + 3].toInt();

        if (sat.id <= 0) {
            continue;
        }

        // Track unique satellites
        if (!m_seenSatIds.contains(sat.id)) {
            m_seenSatIds.insert(sat.id);
            ++m_stats.satellitesSeen;
        }

        // Update or append
        bool replaced = false;
        for (auto &existing : m_satellites) {
            if (existing.id == sat.id) {
                existing = sat;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            m_satellites.append(sat);
        }
    }

    emit satellitesUpdated(m_satellites);
}

// ---------------------------------------------------------------------------
// GSA: DOP and Active Satellites
// ---------------------------------------------------------------------------

/** @brief 解析 GSA 语句 @param fields 字段列表 */
void NmeaParser::parseGsa(const QVector<QByteArray> &fields)
{
    // $--GSA,mode1,mode2,sv1..sv12,pdop,hdop,vdop*CS
    if (fields.size() < 18) {
        ++m_stats.parseErrors;
        return;
    }

    // Mark active satellites
    QSet<int> activeIds;
    for (int i = 3; i < 15; ++i) {
        const int prn = fields[i].toInt();
        if (prn > 0) {
            activeIds.insert(prn);
        }
    }
    for (auto &sat : m_satellites) {
        sat.inUse = activeIds.contains(sat.id);
    }

    // DOP values
    m_currentPos.pdop = fields[15].toDouble();
    m_currentPos.hdop = fields[16].toDouble();
    m_currentPos.vdop = fields[17].toDouble();

    emit satellitesUpdated(m_satellites);
}

// ---------------------------------------------------------------------------
// VTG: Track Made Good and Ground Speed
// ---------------------------------------------------------------------------

/** @brief 解析 VTG 语句 @param fields 字段列表 */
void NmeaParser::parseVtg(const QVector<QByteArray> &fields)
{
    // $--VTG,course,T,courseM,speedN,N,speedK,K,posMode*CS
    if (fields.size() < 8) {
        ++m_stats.parseErrors;
        return;
    }
    m_currentPos.course = fields[1].toDouble();
    // Speed in km/h → m/s
    const double speedKmh = fields[7].toDouble();
    m_currentPos.speed = speedKmh / 3.6;
}

// ---------------------------------------------------------------------------
// Coordinate conversion
// ---------------------------------------------------------------------------

/** @brief NMEA ddmm.mmmm 转十进制度 @param raw NMEA 坐标字符串 @param hemisphere 半球(N/S/E/W) @return 十进制度 */
double NmeaParser::nmeaToDecimal(const QByteArray &raw, char hemisphere)
{
    bool ok = false;
    const double val = raw.toDouble(&ok);
    if (!ok || raw.isEmpty()) {
        return 0.0;
    }
    // NMEA format: dddmm.mmmm or ddmm.mmmm
    const int deg = static_cast<int>(val / 100.0);
    const double min = val - deg * 100.0;
    double decimal = deg + min / 60.0;
    if (hemisphere == 'S' || hemisphere == 'W') {
        decimal = -decimal;
    }
    return decimal;
}
