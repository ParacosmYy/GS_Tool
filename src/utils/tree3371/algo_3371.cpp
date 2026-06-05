/**
 * @file algo_3371.cpp
 */
#include "tree3371/algo_3371.h"
QVector<double> algo_3371::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
