/**
 * @file algo_1508.cpp
 * @brief Algorithm module 1508
 */
#include "fft1508/algo_1508.h"
QVector<double> algo_1508::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
