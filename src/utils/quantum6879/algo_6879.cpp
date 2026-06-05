/**
 * @file algo_6879.cpp
 */
#include "quantum6879/algo_6879.h"
QVector<double> algo_6879::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
