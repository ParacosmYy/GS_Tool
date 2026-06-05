/**
 * @file algo_2985.cpp
 */
#include "matrix2985/algo_2985.h"
QVector<double> algo_2985::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
