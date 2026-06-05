/**
 * @file algo_3578.cpp
 */
#include "neural3578/algo_3578.h"
QVector<double> algo_3578::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
