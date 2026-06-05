/**
 * @file algo_802.cpp
 * @brief Algorithm module 802
 */
#include "poly802/algo_802.h"
QVector<double> algo_802::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
