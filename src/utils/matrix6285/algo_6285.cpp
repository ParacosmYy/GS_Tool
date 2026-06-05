/**
 * @file algo_6285.cpp
 */
#include "matrix6285/algo_6285.h"
QVector<double> algo_6285::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
