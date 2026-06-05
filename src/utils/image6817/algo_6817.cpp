/**
 * @file algo_6817.cpp
 */
#include "image6817/algo_6817.h"
QVector<double> algo_6817::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
