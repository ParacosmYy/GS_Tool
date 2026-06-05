/**
 * @file algo_3412.cpp
 */
#include "compress3412/algo_3412.h"
QVector<double> algo_3412::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
