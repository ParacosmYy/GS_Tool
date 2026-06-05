/**
 * @file algo_4345.cpp
 */
#include "matrix4345/algo_4345.h"
QVector<double> algo_4345::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
