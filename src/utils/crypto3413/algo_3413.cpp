/**
 * @file algo_3413.cpp
 */
#include "crypto3413/algo_3413.h"
QVector<double> algo_3413::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
