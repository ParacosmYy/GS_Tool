/**
 * @file algo_7032.cpp
 */
#include "compress7032/algo_7032.h"
QVector<double> algo_7032::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
