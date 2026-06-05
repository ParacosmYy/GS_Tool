/**
 * @file algo_4905.cpp
 */
#include "matrix4905/algo_4905.h"
QVector<double> algo_4905::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
