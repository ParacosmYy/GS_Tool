/**
 * @file algo_3345.cpp
 */
#include "matrix3345/algo_3345.h"
QVector<double> algo_3345::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
