/**
 * @file algo_850.cpp
 * @brief Algorithm module 850
 */
#include "cluster850/algo_850.h"
QVector<double> algo_850::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
