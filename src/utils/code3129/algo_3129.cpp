/**
 * @file algo_3129.cpp
 */
#include "code3129/algo_3129.h"
QVector<double> algo_3129::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
