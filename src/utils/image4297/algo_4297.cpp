/**
 * @file algo_4297.cpp
 */
#include "image4297/algo_4297.h"
QVector<double> algo_4297::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
