/**
 * @file algo_6279.cpp
 */
#include "quantum6279/algo_6279.h"
QVector<double> algo_6279::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
