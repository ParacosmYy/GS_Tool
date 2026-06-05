/**
 * @file algo_2835.cpp
 */
#include "optim2835/algo_2835.h"
QVector<double> algo_2835::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
