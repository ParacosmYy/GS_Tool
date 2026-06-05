/**
 * @file algo_6763.cpp
 */
#include "string6763/algo_6763.h"
QVector<double> algo_6763::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
