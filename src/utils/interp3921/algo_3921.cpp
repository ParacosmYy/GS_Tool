/**
 * @file algo_3921.cpp
 */
#include "interp3921/algo_3921.h"
QVector<double> algo_3921::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
