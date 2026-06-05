/**
 * @file algo_6409.cpp
 */
#include "code6409/algo_6409.h"
QVector<double> algo_6409::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
