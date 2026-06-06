/**
 * @file algo_6913.cpp
 */
#include "crypto6913/algo_6913.h"
QVector<double> algo_6913::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
