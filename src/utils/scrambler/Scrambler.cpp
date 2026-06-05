/**
 * @file Scrambler.cpp
 * @brief 数据扰码器实现 — LFSR扰码/解扰
 */

#include "utils/scrambler/Scrambler.h"

#include <QElapsedTimer>

Scrambler::Scrambler(quint32 polynomial, quint32 initialState, QObject* parent)
    : QObject(parent), m_polynomial(polynomial),
      m_initialState(initialState), m_lfsrState(initialState), m_timeSum(0.0) {}

QByteArray Scrambler::scramble(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    reset();
    QByteArray result(data.size(), Qt::Uninitialized);

    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ static_cast<char>(nextByte());
    }

    m_stats.totalOperations++;
    m_stats.totalBytesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(data.size(), true);
    return result;
}

QByteArray Scrambler::descramble(const QByteArray& data)
{
    /* 自同步扰码: 解扰与扰码操作相同 */
    QElapsedTimer timer;
    timer.start();

    reset();
    QByteArray result(data.size(), Qt::Uninitialized);

    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ static_cast<char>(nextByte());
    }

    m_stats.totalOperations++;
    m_stats.totalBytesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(data.size(), false);
    return result;
}

void Scrambler::setPolynomial(quint32 poly) { m_polynomial = poly; }
void Scrambler::setInitialState(quint32 state) { m_initialState = state; m_lfsrState = state; }

void Scrambler::reset() { m_lfsrState = m_initialState; }

QByteArray Scrambler::generateSequence(int length)
{
    reset();
    QByteArray seq(length, Qt::Uninitialized);
    for (int i = 0; i < length; ++i) {
        seq[i] = static_cast<char>(nextByte());
    }
    return seq;
}

quint8 Scrambler::nextByte()
{
    /* 多项式至少需要2位才有效 */
    if (m_polynomial <= 1) return 0;

    quint8 result = 0;
    for (int bit = 0; bit < 8; ++bit) {
        /* 找到最高有效位位置 */
        int msb = 0;
        quint32 tmp = m_polynomial;
        while (tmp >>= 1) ++msb;

        /* 反馈位 = 多项式 taps 的异或 */
        quint32 feedback = 0;
        quint32 taps = m_polynomial & ((1U << msb) - 1);
        quint32 state = m_lfsrState & ((1U << msb) - 1);
        for (int k = 0; k < msb; ++k) {
            if (taps & (1U << k)) {
                feedback ^= (state >> k) & 1;
            }
        }

        result = (result << 1) | (m_lfsrState & 1);
        m_lfsrState = ((m_lfsrState >> 1) | (feedback << (msb - 1))) & ((1U << msb) - 1);
    }
    return result;
}

void Scrambler::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
