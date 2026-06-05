/**
 * @file algo_5017.cpp
 */
#include "image5017/algo_5017.h"
QVector<double> algo_5017::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
