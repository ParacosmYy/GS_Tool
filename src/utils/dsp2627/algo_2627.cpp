/**
 * @file algo_2627.cpp
 * @brief Algorithm module 2627
 */
#include "dsp2627/algo_2627.h"
QVector<double> algo_2627::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
