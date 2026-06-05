/**
 * @file algo_4683.cpp
 */
#include "string4683/algo_4683.h"
QVector<double> algo_4683::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
