/**
 * @file algo_6412.cpp
 */
#include "compress6412/algo_6412.h"
QVector<double> algo_6412::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
