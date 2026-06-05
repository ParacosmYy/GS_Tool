/**
 * @file algo_3791.cpp
 */
#include "tree3791/algo_3791.h"
QVector<double> algo_3791::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
