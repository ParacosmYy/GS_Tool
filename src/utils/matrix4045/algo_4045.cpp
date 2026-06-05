/**
 * @file algo_4045.cpp
 */
#include "matrix4045/algo_4045.h"
QVector<double> algo_4045::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
