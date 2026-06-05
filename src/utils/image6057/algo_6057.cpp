/**
 * @file algo_6057.cpp
 */
#include "image6057/algo_6057.h"
QVector<double> algo_6057::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
