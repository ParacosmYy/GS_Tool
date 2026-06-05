/**
 * @file algo_6303.cpp
 */
#include "string6303/algo_6303.h"
QVector<double> algo_6303::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
