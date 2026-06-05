/**
 * @file algo_6589.cpp
 */
#include "code6589/algo_6589.h"
QVector<double> algo_6589::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
