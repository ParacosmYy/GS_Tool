/**
 * @file algo_2787.cpp
 * @brief Algorithm module 2787
 */
#include "dsp2787/algo_2787.h"
QVector<double> algo_2787::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
