/**
 * @file algo_4245.cpp
 */
#include "matrix4245/algo_4245.h"
QVector<double> algo_4245::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
