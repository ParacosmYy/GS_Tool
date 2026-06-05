/**
 * @file algo_1647.cpp
 * @brief Algorithm module 1647
 */
#include "dsp1647/algo_1647.h"
QVector<double> algo_1647::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
