// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPixmap>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

class HeatmapWidget : public QWidget {
    Q_OBJECT
public:
    explicit HeatmapWidget(QWidget *parent = nullptr);
    ~HeatmapWidget() override;

    void setData(const QVector<QVector<double>> &data);
    void setColorRange(double min, double max);
    void setCellSize(int size);
    void setShowValues(bool show);
    void setAutoScale(bool enabled);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void cellHovered(int row, int col, double value);
    void cellClicked(int row, int col, double value);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void updatePixmap();
    QColor valueToColor(double value) const;
    QString formatValue(double value) const;

    QVector<QVector<double>> m_data;
    double m_minValue = 0.0;
    double m_maxValue = 1.0;
    int m_cellSize = 20;
    bool m_showValues = false;
    bool m_autoScale = true;
    int m_hoverRow = -1;
    int m_hoverCol = -1;
    QPixmap m_cache;
    bool m_dirty = true;
};