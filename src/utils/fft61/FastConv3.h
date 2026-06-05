#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class FastConv3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalConvolutions = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit FastConv3(QObject* parent = nullptr);
    void setKernel(const QVector<double>& kernel);
    void setMode(const QString& mode);
    QVector<double> process(const QVector<double>& input);
    void setBlockSize(int size);
    int kernelSize() const { return m_kernel.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void convolutionCompleted(int inLen, int outLen);
private:
    QVector<double> m_kernel; QString m_mode = "full"; int m_blockSize = 4096;
    QVector<double> fftConvolve(const QVector<double>& a, const QVector<double>& b);
    int nextPow2(int n) const;
    Stats m_stats; double m_timeSum = 0.0;
};
