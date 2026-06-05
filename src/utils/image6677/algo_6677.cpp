/**
 * @file algo_6677.cpp
 */
#include "image6677/algo_6677.h"
QVector<double> algo_6677::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
