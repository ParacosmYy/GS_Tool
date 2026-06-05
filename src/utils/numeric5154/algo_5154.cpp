/**
 * @file algo_5154.cpp
 */
#include "numeric5154/algo_5154.h"
QVector<double> algo_5154::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
