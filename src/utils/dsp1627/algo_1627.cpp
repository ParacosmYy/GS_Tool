/**
 * @file algo_1627.cpp
 * @brief Algorithm module 1627
 */
#include "dsp1627/algo_1627.h"
QVector<double> algo_1627::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
