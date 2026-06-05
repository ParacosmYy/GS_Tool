/**
 * @file algo_6673.cpp
 */
#include "crypto6673/algo_6673.h"
QVector<double> algo_6673::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
