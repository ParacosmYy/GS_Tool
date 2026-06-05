/**
 * @file algo_4065.cpp
 */
#include "matrix4065/algo_4065.h"
QVector<double> algo_4065::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
