/**
 * @file algo_821.cpp
 * @brief Algorithm module 821
 */
#include "interp821/algo_821.h"
QVector<double> algo_821::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
