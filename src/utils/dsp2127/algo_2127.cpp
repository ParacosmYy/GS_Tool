/**
 * @file algo_2127.cpp
 * @brief Algorithm module 2127
 */
#include "dsp2127/algo_2127.h"
QVector<double> algo_2127::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
