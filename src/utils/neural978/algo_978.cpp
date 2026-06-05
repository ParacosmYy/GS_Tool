/**
 * @file algo_978.cpp
 * @brief Algorithm module 978
 */
#include "neural978/algo_978.h"
QVector<double> algo_978::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
