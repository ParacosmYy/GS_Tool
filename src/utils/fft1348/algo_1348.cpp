/**
 * @file algo_1348.cpp
 * @brief Algorithm module 1348
 */
#include "fft1348/algo_1348.h"
QVector<double> algo_1348::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
