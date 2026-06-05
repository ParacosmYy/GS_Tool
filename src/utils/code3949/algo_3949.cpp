/**
 * @file algo_3949.cpp
 */
#include "code3949/algo_3949.h"
QVector<double> algo_3949::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
