/**
 * @file algo_6917.cpp
 */
#include "image6917/algo_6917.h"
QVector<double> algo_6917::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
