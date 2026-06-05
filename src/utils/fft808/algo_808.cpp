/**
 * @file algo_808.cpp
 * @brief Algorithm module 808
 */
#include "fft808/algo_808.h"
QVector<double> algo_808::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
