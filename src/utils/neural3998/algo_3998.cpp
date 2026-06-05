/**
 * @file algo_3998.cpp
 */
#include "neural3998/algo_3998.h"
QVector<double> algo_3998::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
