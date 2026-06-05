/**
 * @file algo_1947.cpp
 * @brief Algorithm module 1947
 */
#include "dsp1947/algo_1947.h"
QVector<double> algo_1947::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
