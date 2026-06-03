// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPixmap>
#include <QTimer>

class QPaintEvent;
class QResizeEvent;

class WaterfallWidget : public QWidget {
    Q_OBJECT
public:
    explicit WaterfallWidget(QWidget *parent = nullptr);
    ~WaterfallWidget() override;

    void addSpectrum(const QVector<double> &spectrum);
    void setMaxLines(int lines);
    void setColorRange(double min, double max);
    void setScrollSpeed(int ms);
    void clear();
    void pause();
    void resume();

    int maxLines() const { return m_maxLines; }
    bool isPaused() const { return m_paused; }

signals:
    void spectrumAdded(int lineCount);
    void valueAtCursor(int index, double value);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void scrollImage();
    QColor valueToColor(double value) const;

    QVector<QVector<double>> m_history;
    int m_maxLines = 200;
    double m_minValue = -100.0;
    double m_maxValue = 0.0;
    bool m_paused = false;
    int m_scrollSpeed = 50;
    QPixmap m_waterfall;
    int m_currentLine = 0;
    QTimer m_scrollTimer;
};