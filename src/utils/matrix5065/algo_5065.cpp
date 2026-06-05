/**
 * @file algo_5065.cpp
 */
#include "matrix5065/algo_5065.h"
QVector<double> algo_5065::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
