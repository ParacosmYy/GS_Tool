/**
 * @file algo_2515.cpp
 * @brief Algorithm module 2515
 */
#include "optim2515/algo_2515.h"
QVector<double> algo_2515::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
