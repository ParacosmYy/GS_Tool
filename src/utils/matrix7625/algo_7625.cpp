/**
 * @file algo_7625.cpp
 */
#include "matrix7625/algo_7625.h"
QVector<double> algo_7625::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
