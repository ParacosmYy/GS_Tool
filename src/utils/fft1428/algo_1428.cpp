/**
 * @file algo_1428.cpp
 * @brief Algorithm module 1428
 */
#include "fft1428/algo_1428.h"
QVector<double> algo_1428::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
