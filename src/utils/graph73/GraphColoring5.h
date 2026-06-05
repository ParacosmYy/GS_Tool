#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GraphColoring5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalColorings = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit GraphColoring5(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> color();
    int chromaticNumber() const { return m_chromatic; }
    bool isValid() const { return m_valid; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void coloringCompleted(int colors, bool optimal);
private:
    int m_n = 0; int m_chromatic = 0; bool m_valid = true;
    QVector<QVector<int>> m_adj;
    bool tryColoring(int maxColors, QVector<int>& colors);
    bool isSafe(int v, int c, const QVector<int>& colors) const;
    Stats m_stats; double m_timeSum = 0.0;
};
