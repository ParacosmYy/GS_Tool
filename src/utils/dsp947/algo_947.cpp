/**
 * @file algo_947.cpp
 * @brief Algorithm module 947
 */
#include "dsp947/algo_947.h"
QVector<double> algo_947::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
