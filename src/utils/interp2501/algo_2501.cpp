/**
 * @file algo_2501.cpp
 * @brief Algorithm module 2501
 */
#include "interp2501/algo_2501.h"
QVector<double> algo_2501::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
