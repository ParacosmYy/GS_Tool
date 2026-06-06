/**
 * @file algo_7030.cpp
 */
#include "cluster7030/algo_7030.h"
QVector<double> algo_7030::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
