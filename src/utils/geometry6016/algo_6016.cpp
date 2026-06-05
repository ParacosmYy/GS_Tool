/**
 * @file algo_6016.cpp
 */
#include "geometry6016/algo_6016.h"
QVector<double> algo_6016::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
