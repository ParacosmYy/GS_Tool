/**
 * @file algo_2921.cpp
 */
#include "interp2921/algo_2921.h"
QVector<double> algo_2921::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
