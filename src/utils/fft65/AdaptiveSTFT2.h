#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class AdaptiveSTFT2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit AdaptiveSTFT2(QObject* parent = nullptr);
    void setMinWindowSize(int n);
    void setMaxWindowSize(int n);
    void setAdaptationMode(const QString& mode);
    QVector<QVector<double>> forward(const QVector<double>& signal);
    QVector<double> inverse(const QVector<QVector<double>>& frames);
    QVector<int> windowSizes() const { return m_winSizes; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int frames, int avgWinSize);
private:
    int m_minWin = 256; int m_maxWin = 4096; QString m_mode = "energy";
    QVector<int> m_winSizes;
    int chooseWindowSize(const QVector<double>& frame);
    Stats m_stats; double m_timeSum = 0.0;
};
