/**
 * @file algo_1188.cpp
 * @brief Algorithm module 1188
 */
#include "fft1188/algo_1188.h"
QVector<double> algo_1188::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
