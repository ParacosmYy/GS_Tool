/**
 * @file algo_5521.cpp
 */
#include "interp5521/algo_5521.h"
QVector<double> algo_5521::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
