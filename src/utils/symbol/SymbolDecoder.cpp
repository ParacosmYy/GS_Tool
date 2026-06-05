/**
 * @file SymbolDecoder.cpp
 * @brief 符号解码引擎实现 — NRZ/NRZI/Manchester/差分/RZ解码
 */

#include "utils/symbol/SymbolDecoder.h"

#include <QtMath>

SymbolDecoder::SymbolDecoder(QObject* parent)
    : QObject(parent), m_encoding(Encoding::NRZ), m_threshold(0.5) {}

void SymbolDecoder::setEncoding(Encoding enc) { m_encoding = enc; }
void SymbolDecoder::setThreshold(double thr) { m_threshold = thr; }

/** @brief 解码 @param data 信号 @return 比特 */
QVector<int> SymbolDecoder::decode(const QVector<double>& data)
{
    if (data.isEmpty()) return {};

    QVector<int> bits;
    switch (m_encoding) {
    case Encoding::NRZ:           bits = decodeNRZ(data); break;
    case Encoding::NRZI:          bits = decodeNRZI(data); break;
    case Encoding::Manchester:    bits = decodeManchester(data); break;
    case Encoding::DiffManchester:bits = decodeDiffManchester(data); break;
    case Encoding::RZ:            bits = decodeRZ(data); break;
    }

    m_stats.totalBitsDecoded += static_cast<quint64>(bits.size());
    emit decoded(bits.size());
    return bits;
}

/** @brief 时钟恢复 @param data 信号 @return 边沿位置 */
QVector<int> SymbolDecoder::recoverClock(const QVector<double>& data)
{
    QVector<int> edges;
    for (int i = 1; i < data.size(); ++i) {
        if ((data[i] >= m_threshold) != (data[i - 1] >= m_threshold)) {
            edges.append(i);
        }
    }
    return edges;
}

void SymbolDecoder::resetStatistics() { m_stats = Stats{}; }

/** @brief NRZ解码 @param data 信号 @return 比特 */
QVector<int> SymbolDecoder::decodeNRZ(const QVector<double>& data) const
{
    QVector<int> bits;
    bits.reserve(data.size());
    for (double v : data) {
        bits.append(v >= m_threshold ? 1 : 0);
    }
    return bits;
}

/** @brief NRZI解码 @param data 信号 @return 比特 */
QVector<int> SymbolDecoder::decodeNRZI(const QVector<double>& data) const
{
    QVector<int> bits;
    int prev = 0;
    for (double v : data) {
        int cur = v >= m_threshold ? 1 : 0;
        bits.append(cur != prev ? 1 : 0);
        prev = cur;
    }
    return bits;
}

/** @brief Manchester解码 @param data 信号 @return 比特 */
QVector<int> SymbolDecoder::decodeManchester(const QVector<double>& data) const
{
    QVector<int> bits;
    for (int i = 0; i + 1 < data.size(); i += 2) {
        double first = data[i];
        double second = data[i + 1];
        /* 低→高 = 1, 高→低 = 0 (IEEE 802.3) */
        bits.append(first < second ? 1 : 0);
    }
    return bits;
}

/** @brief 差分Manchester解码 @param data 信号 @return 比特 */
QVector<int> SymbolDecoder::decodeDiffManchester(const QVector<double>& data) const
{
    QVector<int> bits;
    bool prevTransition = false;
    for (int i = 0; i + 1 < data.size(); i += 2) {
        bool transition = data[i] < data[i + 1];
        bits.append(transition == prevTransition ? 0 : 1);
        prevTransition = transition;
    }
    return bits;
}

/** @brief RZ解码 @param data 信号 @return 比特 */
QVector<int> SymbolDecoder::decodeRZ(const QVector<double>& data) const
{
    QVector<int> bits;
    for (int i = 0; i < data.size(); ++i) {
        bits.append(data[i] >= m_threshold ? 1 : 0);
    }
    return bits;
}
