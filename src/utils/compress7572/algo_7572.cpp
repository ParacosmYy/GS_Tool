/**
 * @file algo_7572.cpp
 */
#include "compress7572/algo_7572.h"
QVector<double> algo_7572::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
