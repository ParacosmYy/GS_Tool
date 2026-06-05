/**
 * @file algo_6792.cpp
 */
#include "compress6792/algo_6792.h"
QVector<double> algo_6792::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
