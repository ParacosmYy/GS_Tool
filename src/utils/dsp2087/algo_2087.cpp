/**
 * @file algo_2087.cpp
 * @brief Algorithm module 2087
 */
#include "dsp2087/algo_2087.h"
QVector<double> algo_2087::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
