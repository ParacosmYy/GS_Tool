/**
 * @file algo_2507.cpp
 * @brief Algorithm module 2507
 */
#include "dsp2507/algo_2507.h"
QVector<double> algo_2507::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
