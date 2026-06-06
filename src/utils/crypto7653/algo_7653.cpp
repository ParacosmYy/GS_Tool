/**
 * @file algo_7653.cpp
 */
#include "crypto7653/algo_7653.h"
QVector<double> algo_7653::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
