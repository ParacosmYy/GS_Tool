/**
 * @file algo_3931.cpp
 */
#include "tree3931/algo_3931.h"
QVector<double> algo_3931::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
