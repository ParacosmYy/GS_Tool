/**
 * @file algo_6963.cpp
 */
#include "string6963/algo_6963.h"
QVector<double> algo_6963::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
