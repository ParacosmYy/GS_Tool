/**
 * @file algo_7105.cpp
 */
#include "matrix7105/algo_7105.h"
QVector<double> algo_7105::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
