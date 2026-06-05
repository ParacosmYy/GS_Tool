/**
 * @file algo_6403.cpp
 */
#include "string6403/algo_6403.h"
QVector<double> algo_6403::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
