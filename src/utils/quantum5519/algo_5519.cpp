/**
 * @file algo_5519.cpp
 */
#include "quantum5519/algo_5519.h"
QVector<double> algo_5519::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
