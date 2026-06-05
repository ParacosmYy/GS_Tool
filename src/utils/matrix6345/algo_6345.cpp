/**
 * @file algo_6345.cpp
 */
#include "matrix6345/algo_6345.h"
QVector<double> algo_6345::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
