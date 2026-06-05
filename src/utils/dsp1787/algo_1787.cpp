/**
 * @file algo_1787.cpp
 * @brief Algorithm module 1787
 */
#include "dsp1787/algo_1787.h"
QVector<double> algo_1787::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
