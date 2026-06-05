/**
 * @file algo_5265.cpp
 */
#include "matrix5265/algo_5265.h"
QVector<double> algo_5265::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
