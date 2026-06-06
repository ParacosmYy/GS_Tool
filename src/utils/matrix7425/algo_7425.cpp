/**
 * @file algo_7425.cpp
 */
#include "matrix7425/algo_7425.h"
QVector<double> algo_7425::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
