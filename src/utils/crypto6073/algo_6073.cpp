/**
 * @file algo_6073.cpp
 */
#include "crypto6073/algo_6073.h"
QVector<double> algo_6073::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
