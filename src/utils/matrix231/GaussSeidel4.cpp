/**
 * @file GaussSeidel4.cpp
 * @brief GaussSeidel4 实现
 *
 * 实现多色排序Gauss-Seidel与异步混沌迭代求解稀疏线性系统。
 */

#include "utils/matrix231/GaussSeidel4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussSeidel4::GaussSeidel4(QObject *parent) : QObject(parent) {}
GaussSeidel4::~GaussSeidel4() = default;

/* ---- Configuration ---- */

void GaussSeidel4::setParameters(int maxIterations, double tolerance,
                                   Mode mode)
{
    m_maxIter = qMax(1, maxIterations);
    m_tolerance = qMax(1e-15, tolerance);
    m_mode = mode;
}

/* ---- Set system ---- */

void GaussSeidel4::setSystem(int n,
                               const QVector<double>& values,
                               const QVector<int>& colIndices,
                               const QVector<int>& rowPtr,
                               const QVector<double>& rhs)
{
    m_n = n;
    m_values = values;
    m_colIdx = colIndices;
    m_rowPtr = rowPtr;
    m_rhs = rhs;

    m_stats.matrixSize = n;
    m_stats.numNonZeros = values.size();

    // Compute coloring for multicolor mode
    m_colorOf = computeColoring();
}

/* ---- Compute graph coloring ---- */

QVector<int> GaussSeidel4::computeColoring() const
{
    QVector<int> color(m_n, -1);
    int maxColor = 0;

    for (int i = 0; i < m_n; ++i) {
        // Find colors used by neighbors
        bool usedColor[64] = {};
        for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k) {
            int j = m_colIdx[k];
            if (j != i && color[j] >= 0 && color[j] < 64)
                usedColor[color[j]] = true;
        }
        // Assign smallest available color
        int c = 0;
        while (c < 64 && usedColor[c]) c++;
        color[i] = c;
        maxColor = qMax(maxColor, c);
    }

    // Build color groups
    const_cast<GaussSeidel4*>(this)->m_numColors = maxColor + 1;
    const_cast<GaussSeidel4*>(this)->m_colorGroups.resize(m_numColors);
    for (int i = 0; i < m_n; ++i)
        const_cast<GaussSeidel4*>(this)->m_colorGroups[color[i]].append(i);
    const_cast<GaussSeidel4*>(this)->m_stats.numColors = m_numColors;

    return color;
}

/* ---- Standard GS sweep ---- */

void GaussSeidel4::sweepStandard(QVector<double>& x)
{
    for (int i = 0; i < m_n; ++i) {
        double sigma = 0.0;
        double diag = 1.0;
        for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k) {
            int j = m_colIdx[k];
            if (j == i)
                diag = m_values[k];
            else
                sigma += m_values[k] * x[j];
        }
        if (qAbs(diag) > 1e-15)
            x[i] = (m_rhs[i] - sigma) / diag;
    }
}

/* ---- Multicolor sweep ---- */

void GaussSeidel4::sweepMulticolor(QVector<double>& x)
{
    // Within each color group, updates are independent
    for (int c = 0; c < m_numColors; ++c) {
        for (int i : m_colorGroups[c]) {
            double sigma = 0.0;
            double diag = 1.0;
            for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k) {
                int j = m_colIdx[k];
                if (j == i)
                    diag = m_values[k];
                else
                    sigma += m_values[k] * x[j];
            }
            if (qAbs(diag) > 1e-15)
                x[i] = (m_rhs[i] - sigma) / diag;
        }
    }
}

/* ---- Chaotic sweep ---- */

void GaussSeidel4::sweepChaotic(QVector<double>& x)
{
    // Chaotic iteration: process random subsets without synchronization
    for (int batch = 0; batch < 4; ++batch) {
        int start = (batch * m_n) / 4;
        int end = ((batch + 1) * m_n) / 4;
        for (int i = start; i < end; ++i) {
            double sigma = 0.0;
            double diag = 1.0;
            for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k) {
                int j = m_colIdx[k];
                if (j == i)
                    diag = m_values[k];
                else
                    sigma += m_values[k] * x[j];
            }
            if (qAbs(diag) > 1e-15)
                x[i] = (m_rhs[i] - sigma) / diag;
        }
    }
}

/* ---- Residual ---- */

double GaussSeidel4::residual(const QVector<double>& x) const
{
    double res = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double ax = 0.0;
        for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k)
            ax += m_values[k] * x[m_colIdx[k]];
        double r = m_rhs[i] - ax;
        res += r * r;
    }
    return qSqrt(res);
}

/* ---- Solve ---- */

QVector<double> GaussSeidel4::solve()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        switch (m_mode) {
        case Standard:   sweepStandard(x);   break;
        case Multicolor: sweepMulticolor(x); break;
        case Chaotic:    sweepChaotic(x);     break;
        }

        double res = residual(x);
        m_stats.iterationsUsed = iter + 1;
        m_stats.finalResidual = res;

        if (iter % 50 == 0)
            emit iterationCompleted(iter, res, timer.elapsed());

        if (res < m_tolerance) break;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit iterationCompleted(
        m_stats.iterationsUsed, m_stats.finalResidual, timer.elapsed());
    return x;
}

/* ---- Reset ---- */

void GaussSeidel4::resetStatistics()
{
    m_values.clear();
    m_colIdx.clear();
    m_rowPtr.clear();
    m_rhs.clear();
    m_colorOf.clear();
    m_colorGroups.clear();
    m_n = 0;
    m_numColors = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
