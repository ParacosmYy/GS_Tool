/**
 * @file algo_1887.cpp
 * @brief Algorithm module 1887
 */
#include "dsp1887/algo_1887.h"
QVector<double> algo_1887::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
