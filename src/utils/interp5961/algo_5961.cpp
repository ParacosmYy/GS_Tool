/**
 * @file algo_5961.cpp
 */
#include "interp5961/algo_5961.h"
QVector<double> algo_5961::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
