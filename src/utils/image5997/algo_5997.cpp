/**
 * @file algo_5997.cpp
 */
#include "image5997/algo_5997.h"
QVector<double> algo_5997::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
