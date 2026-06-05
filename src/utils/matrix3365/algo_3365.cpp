/**
 * @file algo_3365.cpp
 */
#include "matrix3365/algo_3365.h"
QVector<double> algo_3365::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
