/**
 * @file algo_3540.cpp
 */
#include "sort3540/algo_3540.h"
QVector<double> algo_3540::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
