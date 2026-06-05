/**
 * @file algo_3697.cpp
 */
#include "image3697/algo_3697.h"
QVector<double> algo_3697::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
