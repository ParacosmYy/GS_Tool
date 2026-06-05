/**
 * @file algo_6245.cpp
 */
#include "matrix6245/algo_6245.h"
QVector<double> algo_6245::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
