/**
 * @file algo_4342.cpp
 */
#include "poly4342/algo_4342.h"
QVector<double> algo_4342::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
