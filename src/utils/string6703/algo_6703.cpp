/**
 * @file algo_6703.cpp
 */
#include "string6703/algo_6703.h"
QVector<double> algo_6703::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
