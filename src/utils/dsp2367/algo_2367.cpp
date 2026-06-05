/**
 * @file algo_2367.cpp
 * @brief Algorithm module 2367
 */
#include "dsp2367/algo_2367.h"
QVector<double> algo_2367::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
