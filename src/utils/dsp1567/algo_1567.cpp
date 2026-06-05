/**
 * @file algo_1567.cpp
 * @brief Algorithm module 1567
 */
#include "dsp1567/algo_1567.h"
QVector<double> algo_1567::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
