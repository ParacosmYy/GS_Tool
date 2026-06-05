/**
 * @file algo_3721.cpp
 */
#include "interp3721/algo_3721.h"
QVector<double> algo_3721::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
