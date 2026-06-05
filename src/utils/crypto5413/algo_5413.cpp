/**
 * @file algo_5413.cpp
 */
#include "crypto5413/algo_5413.h"
QVector<double> algo_5413::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
