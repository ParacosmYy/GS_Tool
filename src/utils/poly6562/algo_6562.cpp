/**
 * @file algo_6562.cpp
 */
#include "poly6562/algo_6562.h"
QVector<double> algo_6562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
