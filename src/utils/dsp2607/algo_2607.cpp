/**
 * @file algo_2607.cpp
 * @brief Algorithm module 2607
 */
#include "dsp2607/algo_2607.h"
QVector<double> algo_2607::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
