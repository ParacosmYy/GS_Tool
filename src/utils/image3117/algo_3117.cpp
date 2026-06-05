/**
 * @file algo_3117.cpp
 */
#include "image3117/algo_3117.h"
QVector<double> algo_3117::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
