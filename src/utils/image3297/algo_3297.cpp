/**
 * @file algo_3297.cpp
 */
#include "image3297/algo_3297.h"
QVector<double> algo_3297::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
