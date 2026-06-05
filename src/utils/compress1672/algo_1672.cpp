/**
 * @file algo_1672.cpp
 * @brief Algorithm module 1672
 */
#include "compress1672/algo_1672.h"
QVector<double> algo_1672::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
