/**
 * @file algo_5753.cpp
 */
#include "crypto5753/algo_5753.h"
QVector<double> algo_5753::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
