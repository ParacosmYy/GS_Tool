/**
 * @file algo_5573.cpp
 */
#include "crypto5573/algo_5573.h"
QVector<double> algo_5573::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
