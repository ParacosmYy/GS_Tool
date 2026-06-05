/**
 * @file algo_3385.cpp
 */
#include "matrix3385/algo_3385.h"
QVector<double> algo_3385::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
