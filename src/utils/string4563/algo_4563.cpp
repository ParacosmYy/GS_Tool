/**
 * @file algo_4563.cpp
 */
#include "string4563/algo_4563.h"
QVector<double> algo_4563::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
