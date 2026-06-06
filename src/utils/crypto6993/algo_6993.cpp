/**
 * @file algo_6993.cpp
 */
#include "crypto6993/algo_6993.h"
QVector<double> algo_6993::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
