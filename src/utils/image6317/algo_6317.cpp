/**
 * @file algo_6317.cpp
 */
#include "image6317/algo_6317.h"
QVector<double> algo_6317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
