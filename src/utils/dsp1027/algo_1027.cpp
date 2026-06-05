/**
 * @file algo_1027.cpp
 * @brief Algorithm module 1027
 */
#include "dsp1027/algo_1027.h"
QVector<double> algo_1027::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
