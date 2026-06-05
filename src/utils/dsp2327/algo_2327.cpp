/**
 * @file algo_2327.cpp
 * @brief Algorithm module 2327
 */
#include "dsp2327/algo_2327.h"
QVector<double> algo_2327::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
