/**
 * @file algo_5225.cpp
 */
#include "matrix5225/algo_5225.h"
QVector<double> algo_5225::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
