/**
 * @file algo_6085.cpp
 */
#include "matrix6085/algo_6085.h"
QVector<double> algo_6085::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
