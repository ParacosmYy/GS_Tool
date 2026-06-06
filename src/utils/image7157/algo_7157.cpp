/**
 * @file algo_7157.cpp
 */
#include "image7157/algo_7157.h"
QVector<double> algo_7157::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
