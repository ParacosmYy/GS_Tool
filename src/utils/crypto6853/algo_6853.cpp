/**
 * @file algo_6853.cpp
 */
#include "crypto6853/algo_6853.h"
QVector<double> algo_6853::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
