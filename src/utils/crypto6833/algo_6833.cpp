/**
 * @file algo_6833.cpp
 */
#include "crypto6833/algo_6833.h"
QVector<double> algo_6833::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
