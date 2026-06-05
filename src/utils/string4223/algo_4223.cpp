/**
 * @file algo_4223.cpp
 */
#include "string4223/algo_4223.h"
QVector<double> algo_4223::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
