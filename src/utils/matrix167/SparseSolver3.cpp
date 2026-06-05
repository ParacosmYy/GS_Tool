/**
 * @file SparseSolver3.cpp
 * @brief Sparse matrix solver using conjugate gradient implementation
 */
#include "matrix167/SparseSolver3.h"
#include <QElapsedTimer>

QVector<double> SparseSolver3::compute(const QVector<double> &input)
{
    QElapsedTimer t;
    t.start();
    m_stats.calls++;

    if (input.isEmpty()) {
        m_stats.errors++;
        return {};
    }

    QVector<double> result = input;
    m_stats.itemsProcessed += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

