/**
 * @file algo_5117.cpp
 */
#include "image5117/algo_5117.h"
QVector<double> algo_5117::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
