/**
 * @file algo_5945.cpp
 */
#include "matrix5945/algo_5945.h"
QVector<double> algo_5945::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
