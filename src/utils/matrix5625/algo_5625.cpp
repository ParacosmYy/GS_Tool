/**
 * @file algo_5625.cpp
 */
#include "matrix5625/algo_5625.h"
QVector<double> algo_5625::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
