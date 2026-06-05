/**
 * @file algo_2587.cpp
 * @brief Algorithm module 2587
 */
#include "dsp2587/algo_2587.h"
QVector<double> algo_2587::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
