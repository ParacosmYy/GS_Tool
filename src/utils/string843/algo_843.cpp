/**
 * @file algo_843.cpp
 * @brief Algorithm module 843
 */
#include "string843/algo_843.h"
QVector<double> algo_843::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
