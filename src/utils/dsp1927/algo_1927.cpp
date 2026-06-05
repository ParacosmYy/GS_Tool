/**
 * @file algo_1927.cpp
 * @brief Algorithm module 1927
 */
#include "dsp1927/algo_1927.h"
QVector<double> algo_1927::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
