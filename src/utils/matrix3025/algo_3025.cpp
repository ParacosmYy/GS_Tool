/**
 * @file algo_3025.cpp
 */
#include "matrix3025/algo_3025.h"
QVector<double> algo_3025::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
