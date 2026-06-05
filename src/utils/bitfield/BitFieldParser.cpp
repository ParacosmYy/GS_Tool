/**
 * @file BitFieldParser.cpp
 * @brief 位域解析器实现
 */

#include "utils/bitfield/BitFieldParser.h"
#include <QElapsedTimer>

BitFieldParser::BitFieldParser(QObject* parent)
    : QObject(parent), m_bigEndian(true), m_timeSum(0.0) {}

void BitFieldParser::addField(const FieldDef& def) { m_fields.append(def); }
void BitFieldParser::clearFields() { m_fields.clear(); m_scales.clear(); }
void BitFieldParser::setBigEndian(bool bigEndian) { m_bigEndian = bigEndian; }

void BitFieldParser::setScale(const QString& fieldName, double scale, double offset)
{
    m_scales[fieldName] = {scale, offset};
}

QList<BitFieldParser::FieldValue> BitFieldParser::parse(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<FieldValue> result;
    for (const auto& def : m_fields) {
        quint64 raw = extractBits(data, def.startBit, def.bitWidth);

        FieldValue fv;
        fv.name = def.name;
        fv.rawValue = raw;
        fv.hexString = QStringLiteral("0x") + QString::number(raw, 16).toUpper().rightJustified(
            (def.bitWidth + 3) / 4, '0');

        /* 符号扩展 */
        if (def.isSigned && def.bitWidth < 64) {
            quint64 signBit = 1ULL << (def.bitWidth - 1);
            if (raw & signBit) {
                quint64 mask = (~0ULL) << def.bitWidth;
                raw |= mask;
            }
        }

        /* 缩放 */
        if (m_scales.contains(def.name)) {
            double scale = m_scales[def.name].first;
            double offset = m_scales[def.name].second;
            fv.scaledValue = static_cast<double>(raw) * scale + offset;
        } else {
            fv.scaledValue = static_cast<double>(raw);
        }

        result.append(fv);
        emit fieldExtracted(fv);
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalParses;
    m_stats.totalFieldsExtracted += m_fields.size();
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalParses;

    emit parseComplete(result.size());
    return result;
}

quint64 BitFieldParser::extractBits(const QByteArray& data, int startBit, int bitWidth) const
{
    int totalBits = data.size() * 8;
    if (startBit < 0 || bitWidth <= 0 || startBit + bitWidth > totalBits) return 0;

    quint64 result = 0;
    for (int i = 0; i < bitWidth; ++i) {
        int bitPos = startBit + i;
        int byteIdx = m_bigEndian ? (bitPos / 8) : (data.size() - 1 - bitPos / 8);
        int bitIdx = m_bigEndian ? (7 - bitPos % 8) : (bitPos % 8);

        if (byteIdx >= 0 && byteIdx < data.size()) {
            quint8 byte = static_cast<quint8>(data[byteIdx]);
            if (byte & (1 << bitIdx)) result |= (1ULL << i);
        }
    }
    return result;
}

void BitFieldParser::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
