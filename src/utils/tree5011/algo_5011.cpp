/**
 * @file algo_5011.cpp
 */
#include "tree5011/algo_5011.h"
QVector<double> algo_5011::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
