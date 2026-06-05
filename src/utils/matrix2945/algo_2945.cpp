/**
 * @file algo_2945.cpp
 */
#include "matrix2945/algo_2945.h"
QVector<double> algo_2945::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
