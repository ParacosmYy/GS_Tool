/**
 * @file algo_1848.cpp
 * @brief Algorithm module 1848
 */
#include "fft1848/algo_1848.h"
QVector<double> algo_1848::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
