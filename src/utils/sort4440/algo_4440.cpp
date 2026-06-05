/**
 * @file algo_4440.cpp
 */
#include "sort4440/algo_4440.h"
QVector<double> algo_4440::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
