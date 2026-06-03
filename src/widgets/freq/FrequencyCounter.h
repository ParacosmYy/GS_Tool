#pragma once
#include <QWidget>
#include <QByteArray>
#include <QTimer>
#include <QLabel>

class FrequencyCounter : public QWidget {
    Q_OBJECT
public:
    explicit FrequencyCounter(QWidget *parent = nullptr);
    ~FrequencyCounter() override;
    void feedSample(double value);
    void feedBuffer(const QByteArray &data, int sampleSize = 2);
    void setGateTime(int msecs);
    void setTriggerLevel(double level);
    void reset();
    double frequency() const;
    double period() const;
    int pulseCount() const;
signals:
    void frequencyChanged(double hz);
    void pulseCounted(int count);
protected:
    void setupUi();
private:
    void onGateTimeout();
    QLabel *m_freqLabel = nullptr;
    QLabel *m_countLabel = nullptr;
    double m_frequency = 0.0;
    int m_pulseCount = 0;
    int m_gateTime = 1000;
    double m_triggerLevel = 0.0;
    bool m_lastAbove = false;
    QTimer *m_gateTimer = nullptr;
    qint64 m_gateStart = 0;
};
