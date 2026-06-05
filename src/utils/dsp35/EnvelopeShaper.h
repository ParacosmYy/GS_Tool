#pragma once
#include <QObject>
#include <QVector>
/** @brief Envelope shaper - ADSR/envelope follower/transient enhance */
class EnvelopeShaper : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit EnvelopeShaper(QObject* parent = nullptr);
    void setAttack(double ms); void setDecay(double ms);
    void setSustain(double level); void setRelease(double ms);
    void setSampleRate(double rate);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    QVector<double> envelope() const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingComplete(int samples);
private:
    double m_attack = 10.0; double m_decay = 100.0;
    double m_sustain = 0.5; double m_release = 200.0;
    double m_sampleRate = 44100.0; double m_envelope = 0.0;
    QVector<double> m_envelopeBuf;
    Stats m_stats; double m_timeSum = 0.0;
};
