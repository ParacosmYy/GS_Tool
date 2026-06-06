/**
 * @file algo_7265.cpp
 */
#include "matrix7265/algo_7265.h"
QVector<double> algo_7265::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
