/**
 * @file algo_5394.cpp
 */
#include "numeric5394/algo_5394.h"
QVector<double> algo_5394::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
