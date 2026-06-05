/**
 * @file algo_6431.cpp
 */
#include "tree6431/algo_6431.h"
QVector<double> algo_6431::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
