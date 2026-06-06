/**
 * @file algo_7413.cpp
 */
#include "crypto7413/algo_7413.h"
QVector<double> algo_7413::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
