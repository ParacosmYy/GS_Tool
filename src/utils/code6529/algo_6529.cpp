/**
 * @file algo_6529.cpp
 */
#include "code6529/algo_6529.h"
QVector<double> algo_6529::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
