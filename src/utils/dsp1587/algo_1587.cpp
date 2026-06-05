/**
 * @file algo_1587.cpp
 * @brief Algorithm module 1587
 */
#include "dsp1587/algo_1587.h"
QVector<double> algo_1587::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
