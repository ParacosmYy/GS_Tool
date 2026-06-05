/**
 * @file algo_5885.cpp
 */
#include "matrix5885/algo_5885.h"
QVector<double> algo_5885::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
