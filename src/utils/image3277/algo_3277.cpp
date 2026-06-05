/**
 * @file algo_3277.cpp
 */
#include "image3277/algo_3277.h"
QVector<double> algo_3277::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
