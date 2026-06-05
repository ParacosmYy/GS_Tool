/**
 * @file algo_6743.cpp
 */
#include "string6743/algo_6743.h"
QVector<double> algo_6743::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
