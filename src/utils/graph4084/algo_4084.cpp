/**
 * @file algo_4084.cpp
 */
#include "graph4084/algo_4084.h"
QVector<double> algo_4084::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
