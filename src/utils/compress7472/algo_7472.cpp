/**
 * @file algo_7472.cpp
 */
#include "compress7472/algo_7472.h"
QVector<double> algo_7472::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
