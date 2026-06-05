/**
 * @file algo_1387.cpp
 * @brief Algorithm module 1387
 */
#include "dsp1387/algo_1387.h"
QVector<double> algo_1387::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
