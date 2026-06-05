/**
 * @file algo_6540.cpp
 */
#include "sort6540/algo_6540.h"
QVector<double> algo_6540::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
