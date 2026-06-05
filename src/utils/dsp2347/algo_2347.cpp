/**
 * @file algo_2347.cpp
 * @brief Algorithm module 2347
 */
#include "dsp2347/algo_2347.h"
QVector<double> algo_2347::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
