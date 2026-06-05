/**
 * @file algo_1487.cpp
 * @brief Algorithm module 1487
 */
#include "dsp1487/algo_1487.h"
QVector<double> algo_1487::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
