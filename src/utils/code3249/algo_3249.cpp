/**
 * @file algo_3249.cpp
 */
#include "code3249/algo_3249.h"
QVector<double> algo_3249::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
