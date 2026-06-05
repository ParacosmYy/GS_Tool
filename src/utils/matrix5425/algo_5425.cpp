/**
 * @file algo_5425.cpp
 */
#include "matrix5425/algo_5425.h"
QVector<double> algo_5425::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
