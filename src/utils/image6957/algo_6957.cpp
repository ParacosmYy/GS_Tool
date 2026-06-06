/**
 * @file algo_6957.cpp
 */
#include "image6957/algo_6957.h"
QVector<double> algo_6957::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
