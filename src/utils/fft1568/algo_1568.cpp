/**
 * @file algo_1568.cpp
 * @brief Algorithm module 1568
 */
#include "fft1568/algo_1568.h"
QVector<double> algo_1568::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
