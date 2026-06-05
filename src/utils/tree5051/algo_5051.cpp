/**
 * @file algo_5051.cpp
 */
#include "tree5051/algo_5051.h"
QVector<double> algo_5051::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
