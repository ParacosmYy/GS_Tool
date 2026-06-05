/**
 * @file AVLTree3.cpp
 * @brief AVLTree3 implementation
 */
#include "graph215/AVLTree3.h"
#include <QElapsedTimer>
QVector<double> AVLTree3::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

