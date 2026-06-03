#pragma once
#include <QWidget>
#include <QByteArray>
#include <QVector>

class AudioSpectrum : public QWidget {
    Q_OBJECT
public:
    explicit AudioSpectrum(QWidget *parent = nullptr);
    ~AudioSpectrum() override;
    void setSampleRate(int rate);
    void setFftSize(int size);
    void feedData(const QByteArray &pcmData);
    void setDbRange(double minDb, double maxDb);
    void setBarCount(int bars);
    void setSmoothFactor(double factor);
    int sampleRate() const;
    int fftSize() const;
signals:
    void spectrumUpdated(const QVector<double> &magnitudes);
    void peakFrequencyChanged(double freq);
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    void processFft();
    int m_sampleRate = 44100;
    int m_fftSize = 1024;
    int m_barCount = 32;
    double m_minDb = -80.0;
    double m_maxDb = 0.0;
    double m_smoothFactor = 0.7;
    QVector<double> m_magnitudes;
    QVector<double> m_smoothed;
    QByteArray m_buffer;
};
