/**
 * @file algo_2727.cpp
 * @brief Algorithm module 2727
 */
#include "dsp2727/algo_2727.h"
QVector<double> algo_2727::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
