/**
 * @file algo_3231.cpp
 */
#include "tree3231/algo_3231.h"
QVector<double> algo_3231::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
