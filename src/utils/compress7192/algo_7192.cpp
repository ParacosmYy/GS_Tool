/**
 * @file algo_7192.cpp
 */
#include "compress7192/algo_7192.h"
QVector<double> algo_7192::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
