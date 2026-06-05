/**
 * @file algo_4042.cpp
 */
#include "poly4042/algo_4042.h"
QVector<double> algo_4042::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
