/**
 * @file algo_2207.cpp
 * @brief Algorithm module 2207
 */
#include "dsp2207/algo_2207.h"
QVector<double> algo_2207::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
