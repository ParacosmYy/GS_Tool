/**
 * @file WatershedSegmentation.cpp
 * @brief 分水岭分割实现 — 沉浸模型
 */

#include "utils/watershed/WatershedSegmentation.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>
#include <vector>

constexpr int WatershedSegmentation::dx[4];
constexpr int WatershedSegmentation::dy[4];

WatershedSegmentation::WatershedSegmentation(QObject* parent)
    : QObject(parent) {}

QVector<QVector<int>> WatershedSegmentation::segment(
    const QVector<QVector<double>>& elevation)
{
    QElapsedTimer timer;
    timer.start();

    int rows = elevation.size();
    int cols = (rows > 0) ? elevation[0].size() : 0;
    QVector<QVector<int>> labels(rows, QVector<int>(cols, 0));

    if (rows == 0 || cols == 0) {
        m_regionCount = 0;
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs =
            (m_stats.totalSegmented > 0) ?
            m_timeSum / m_stats.totalSegmented : 0.0;
        return labels;
    }

    auto pixels = collectSortedPixels(elevation);
    const int WSHED = -1;
    int currentLabel = 0;
    std::queue<std::pair<int, int>> queue;
    std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, 0));

    size_t idx = 0;
    while (idx < pixels.size()) {
        double h = pixels[idx].val;
        std::vector<size_t> levelIndices;
        while (idx < pixels.size() &&
               qFuzzyCompare(pixels[idx].val, h)) {
            levelIndices.push_back(idx);
            idx++;
        }

        processLevelNeighbors(pixels, levelIndices, labels,
                              dist, queue, rows, cols, WSHED);
        bfsExpand(labels, dist, queue, rows, cols, WSHED);
        createNewRegions(pixels, levelIndices, labels,
                         dist, currentLabel, rows, cols);
    }

    m_regionCount = currentLabel;
    m_stats.totalSegmented++;
    m_stats.totalRegions += static_cast<quint64>(currentLabel);
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSegmented;

    emit segmentationCompleted(currentLabel);
    return labels;
}

std::vector<WatershedSegmentation::Pixel>
WatershedSegmentation::collectSortedPixels(
    const QVector<QVector<double>>& elevation) const
{
    int rows = elevation.size();
    int cols = elevation[0].size();
    std::vector<Pixel> pixels;
    pixels.reserve(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            pixels.push_back({r, c, elevation[r][c]});
        }
    }
    std::sort(pixels.begin(), pixels.end(),
              [](const Pixel& a, const Pixel& b) { return a.val < b.val; });
    return pixels;
}

void WatershedSegmentation::processLevelNeighbors(
    const std::vector<Pixel>& pixels,
    const std::vector<size_t>& levelIndices,
    QVector<QVector<int>>& labels,
    std::vector<std::vector<int>>& dist,
    std::queue<std::pair<int, int>>& queue,
    int rows, int cols, int wshed) const
{
    for (size_t li : levelIndices) {
        int r = pixels[li].r;
        int c = pixels[li].c;
        dist[r][c] = 1;
        for (int d = 0; d < 4; ++d) {
            int nr = r + dx[d];
            int nc = c + dy[d];
            if (!inBounds(nr, nc, rows, cols)) continue;
            if (labels[nr][nc] > 0 || labels[nr][nc] == wshed) {
                dist[r][c] = 2;
                queue.push({r, c});
                break;
            }
        }
    }
}

void WatershedSegmentation::bfsExpand(
    QVector<QVector<int>>& labels,
    std::vector<std::vector<int>>& dist,
    std::queue<std::pair<int, int>>& queue,
    int rows, int cols, int wshed) const
{
    while (!queue.empty()) {
        auto [cr, cc] = queue.front();
        queue.pop();
        for (int d = 0; d < 4; ++d) {
            int nr = cr + dx[d];
            int nc = cc + dy[d];
            if (!inBounds(nr, nc, rows, cols)) continue;
            if (dist[nr][nc] == 0 && labels[nr][nc] == 0) {
                dist[nr][nc] = dist[cr][cc] + 1;
                if (labels[cr][cc] > 0) {
                    labels[nr][nc] = labels[cr][cc];
                    queue.push({nr, nc});
                } else if (labels[cr][cc] == wshed) {
                    labels[nr][nc] = wshed;
                    queue.push({nr, nc});
                }
            } else if (labels[nr][nc] > 0 && labels[cr][cc] > 0 &&
                       labels[nr][nc] != labels[cr][cc]) {
                labels[cr][cc] = wshed;
            }
        }
    }
}

void WatershedSegmentation::createNewRegions(
    const std::vector<Pixel>& pixels,
    const std::vector<size_t>& levelIndices,
    QVector<QVector<int>>& labels,
    std::vector<std::vector<int>>& dist,
    int& currentLabel, int rows, int cols) const
{
    for (size_t li : levelIndices) {
        int r = pixels[li].r;
        int c = pixels[li].c;
        dist[r][c] = 0;
        if (labels[r][c] == 0) {
            currentLabel++;
            labels[r][c] = currentLabel;
            std::queue<std::pair<int, int>> q;
            q.push({r, c});
            bfsLabelNewRegion(labels, q, currentLabel, rows, cols);
        }
    }
}

void WatershedSegmentation::bfsLabelNewRegion(
    QVector<QVector<int>>& labels,
    std::queue<std::pair<int, int>>& queue,
    int regionId, int rows, int cols) const
{
    while (!queue.empty()) {
        auto [qr, qc] = queue.front();
        queue.pop();
        for (int d = 0; d < 4; ++d) {
            int nqr = qr + dx[d];
            int nqc = qc + dy[d];
            if (!inBounds(nqr, nqc, rows, cols)) continue;
            if (labels[nqr][nqc] == 0) {
                labels[nqr][nqc] = regionId;
                queue.push({nqr, nqc});
            }
        }
    }
}

bool WatershedSegmentation::inBounds(int r, int c, int rows, int cols) const
{
    return r >= 0 && r < rows && c >= 0 && c < cols;
}

void WatershedSegmentation::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}
