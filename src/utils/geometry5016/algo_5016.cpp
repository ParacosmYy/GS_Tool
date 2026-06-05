/**
 * @file algo_5016.cpp
 */
#include "geometry5016/algo_5016.h"
QVector<double> algo_5016::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
