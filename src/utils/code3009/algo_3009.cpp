/**
 * @file algo_3009.cpp
 */
#include "code3009/algo_3009.h"
QVector<double> algo_3009::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
