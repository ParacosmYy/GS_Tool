/**
 * @file algo_3783.cpp
 */
#include "string3783/algo_3783.h"
QVector<double> algo_3783::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
