/**
 * @file algo_4736.cpp
 */
#include "geometry4736/algo_4736.h"
QVector<double> algo_4736::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
