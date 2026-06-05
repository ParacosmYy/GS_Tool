/**
 * @file algo_3183.cpp
 */
#include "string3183/algo_3183.h"
QVector<double> algo_3183::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
