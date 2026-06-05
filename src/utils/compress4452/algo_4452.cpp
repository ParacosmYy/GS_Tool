/**
 * @file algo_4452.cpp
 */
#include "compress4452/algo_4452.h"
QVector<double> algo_4452::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
