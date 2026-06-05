/**
 * @file algo_2027.cpp
 * @brief Algorithm module 2027
 */
#include "dsp2027/algo_2027.h"
QVector<double> algo_2027::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
