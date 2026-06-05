/**
 * @file algo_2287.cpp
 * @brief Algorithm module 2287
 */
#include "dsp2287/algo_2287.h"
QVector<double> algo_2287::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
