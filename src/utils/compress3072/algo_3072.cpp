/**
 * @file algo_3072.cpp
 */
#include "compress3072/algo_3072.h"
QVector<double> algo_3072::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
