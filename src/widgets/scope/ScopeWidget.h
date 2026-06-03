#pragma once
#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QPair>

class ScopeWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScopeWidget(QWidget *parent = nullptr);
    ~ScopeWidget() override;
    void setChannelCount(int count);
    void setSampleBuffer(int size);
    void addSample(int channel, double value);
    void addSamples(int channel, const QVector<double> &values);
    void setTimeScale(double msPerDiv);
    void setVoltageScale(double voltsPerDiv);
    void setTriggerChannel(int ch);
    void setTriggerLevel(double level);
    void setRunning(bool on);
    void clearData();
    int channelCount() const;
    bool isRunning() const;
signals:
    void triggerFired();
    void dataOverflow();
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    void drawGrid(QPainter &p, int w, int h);
    QVector<QVector<double>> m_channels;
    int m_bufferSize = 1024;
    double m_timeScale = 1.0;
    double m_voltScale = 1.0;
    int m_triggerCh = 0;
    double m_triggerLevel = 0.0;
    bool m_running = true;
    int m_writePos = 0;
    static constexpr int kDivisions = 10;
};
