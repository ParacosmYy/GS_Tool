/**
 * @file algo_5233.cpp
 */
#include "crypto5233/algo_5233.h"
QVector<double> algo_5233::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
