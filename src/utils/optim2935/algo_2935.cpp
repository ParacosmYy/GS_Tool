/**
 * @file algo_2935.cpp
 */
#include "optim2935/algo_2935.h"
QVector<double> algo_2935::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
