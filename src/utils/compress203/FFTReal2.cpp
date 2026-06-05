/**
 * @file FFTReal2.cpp
 * @brief FFTReal2 implementation
 */
#include "compress203/FFTReal2.h"
#include <QElapsedTimer>
QVector<double> FFTReal2::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

