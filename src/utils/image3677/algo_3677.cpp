/**
 * @file algo_3677.cpp
 */
#include "image3677/algo_3677.h"
QVector<double> algo_3677::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
