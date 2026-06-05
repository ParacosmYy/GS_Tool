/**
 * @file algo_4721.cpp
 */
#include "interp4721/algo_4721.h"
QVector<double> algo_4721::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
