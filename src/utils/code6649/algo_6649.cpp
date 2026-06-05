/**
 * @file algo_6649.cpp
 */
#include "code6649/algo_6649.h"
QVector<double> algo_6649::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
