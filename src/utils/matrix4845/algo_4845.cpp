/**
 * @file algo_4845.cpp
 */
#include "matrix4845/algo_4845.h"
QVector<double> algo_4845::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
