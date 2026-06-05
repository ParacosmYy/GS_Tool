/**
 * @file algo_2247.cpp
 * @brief Algorithm module 2247
 */
#include "dsp2247/algo_2247.h"
QVector<double> algo_2247::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
