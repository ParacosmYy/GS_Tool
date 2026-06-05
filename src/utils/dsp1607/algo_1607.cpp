/**
 * @file algo_1607.cpp
 * @brief Algorithm module 1607
 */
#include "dsp1607/algo_1607.h"
QVector<double> algo_1607::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
