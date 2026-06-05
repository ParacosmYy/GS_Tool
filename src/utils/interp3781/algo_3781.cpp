/**
 * @file algo_3781.cpp
 */
#include "interp3781/algo_3781.h"
QVector<double> algo_3781::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
