/**
 * @file algo_7423.cpp
 */
#include "string7423/algo_7423.h"
QVector<double> algo_7423::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
