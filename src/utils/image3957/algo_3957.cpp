/**
 * @file algo_3957.cpp
 */
#include "image3957/algo_3957.h"
QVector<double> algo_3957::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
