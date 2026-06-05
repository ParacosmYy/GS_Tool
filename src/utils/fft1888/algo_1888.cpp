/**
 * @file algo_1888.cpp
 * @brief Algorithm module 1888
 */
#include "fft1888/algo_1888.h"
QVector<double> algo_1888::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
