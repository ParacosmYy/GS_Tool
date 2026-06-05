/**
 * @file algo_2567.cpp
 * @brief Algorithm module 2567
 */
#include "dsp2567/algo_2567.h"
QVector<double> algo_2567::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
