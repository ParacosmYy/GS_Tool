/**
 * @file algo_3137.cpp
 */
#include "image3137/algo_3137.h"
QVector<double> algo_3137::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
