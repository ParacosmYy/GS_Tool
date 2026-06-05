/**
 * @file algo_5403.cpp
 */
#include "string5403/algo_5403.h"
QVector<double> algo_5403::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
