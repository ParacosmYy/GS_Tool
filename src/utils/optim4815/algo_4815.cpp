/**
 * @file algo_4815.cpp
 */
#include "optim4815/algo_4815.h"
QVector<double> algo_4815::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
