/**
 * @file algo_5851.cpp
 */
#include "tree5851/algo_5851.h"
QVector<double> algo_5851::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
