/**
 * @file algo_2107.cpp
 * @brief Algorithm module 2107
 */
#include "dsp2107/algo_2107.h"
QVector<double> algo_2107::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
