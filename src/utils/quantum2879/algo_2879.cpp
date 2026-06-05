/**
 * @file algo_2879.cpp
 */
#include "quantum2879/algo_2879.h"
QVector<double> algo_2879::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
