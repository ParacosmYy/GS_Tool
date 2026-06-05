/**
 * @file algo_2387.cpp
 * @brief Algorithm module 2387
 */
#include "dsp2387/algo_2387.h"
QVector<double> algo_2387::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
