/**
 * @file algo_1547.cpp
 * @brief Algorithm module 1547
 */
#include "dsp1547/algo_1547.h"
QVector<double> algo_1547::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
