/**
 * @file algo_5565.cpp
 */
#include "matrix5565/algo_5565.h"
QVector<double> algo_5565::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
