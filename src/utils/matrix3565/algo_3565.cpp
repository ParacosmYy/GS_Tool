/**
 * @file algo_3565.cpp
 */
#include "matrix3565/algo_3565.h"
QVector<double> algo_3565::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
