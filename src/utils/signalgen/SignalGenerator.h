#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QTimer>

class SignalGenerator : public QObject {
    Q_OBJECT
public:
    enum WaveformType { Sine, Square, Triangle, Sawtooth, Noise, Custom };
    Q_ENUM(WaveformType)

    explicit SignalGenerator(QObject *parent = nullptr);
    ~SignalGenerator() override;

    void setWaveform(WaveformType type);
    void setFrequency(double hz);
    void setAmplitude(double amp);
    void setOffset(double offset);
    void setSampleRate(int rate);
    void setDuration(int msecs);
    void setCustomData(const QByteArray &data);

    void start();
    void stop();
    bool isRunning() const;

    double currentValue() const;
    QByteArray generateSamples(int count);
    QByteArray generateDuration(int msecs);

    WaveformType waveform() const;
    double frequency() const;
    double amplitude() const;
    int sampleRate() const;

signals:
    void sampleReady(double value);
    void generationFinished(int sampleCount);
    void error(const QString &msg);

private:
    void onTimerTick();
    double computeSample(int index) const;

    WaveformType m_waveform = Sine;
    double m_frequency = 1.0;
    double m_amplitude = 1.0;
    double m_offset = 0.0;
    int m_sampleRate = 1000;
    int m_duration = 0;
    QByteArray m_customData;
    QTimer *m_timer = nullptr;
    int m_sampleIndex = 0;
    bool m_running = false;
};
