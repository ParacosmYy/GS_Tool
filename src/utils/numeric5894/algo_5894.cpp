/**
 * @file algo_5894.cpp
 */
#include "numeric5894/algo_5894.h"
QVector<double> algo_5894::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
