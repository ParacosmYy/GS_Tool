/**
 * @file algo_1767.cpp
 * @brief Algorithm module 1767
 */
#include "dsp1767/algo_1767.h"
QVector<double> algo_1767::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
