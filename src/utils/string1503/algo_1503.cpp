/**
 * @file algo_1503.cpp
 * @brief Algorithm module 1503
 */
#include "string1503/algo_1503.h"
QVector<double> algo_1503::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
