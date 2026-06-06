/**
 * @file algo_7798.cpp
 */
#include "neural7798/algo_7798.h"
QVector<double> algo_7798::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
