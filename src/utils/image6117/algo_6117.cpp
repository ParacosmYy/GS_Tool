/**
 * @file algo_6117.cpp
 */
#include "image6117/algo_6117.h"
QVector<double> algo_6117::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
