/**
 * @file algo_6603.cpp
 */
#include "string6603/algo_6603.h"
QVector<double> algo_6603::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
