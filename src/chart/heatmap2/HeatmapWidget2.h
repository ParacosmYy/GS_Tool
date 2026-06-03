#pragma once
#include <QWidget>
#include <QVector>
#include <QPair>

class HeatmapWidget : public QWidget {
    Q_OBJECT
public:
    explicit HeatmapWidget(QWidget *parent = nullptr);
    ~HeatmapWidget() override;
    void setData(const QVector<QVector<double>> &matrix);
    void setCellSize(int w, int h);
    void setColorRange(double min, double max);
    void setLabels(const QStringList &rowLabels, const QStringList &colLabels);
    void setGridVisible(bool visible);
    int rowCount() const;
    int colCount() const;
    double valueAt(int row, int col) const;
signals:
    void cellClicked(int row, int col, double value);
    void cellHovered(int row, int col, double value);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    QColor valueToColor(double v) const;
    QPair<int,int> posToCell(const QPoint &pos) const;
    QVector<QVector<double>> m_data;
    int m_cellW = 20;
    int m_cellH = 20;
    double m_minVal = 0.0;
    double m_maxVal = 1.0;
    bool m_gridVisible = true;
    QStringList m_rowLabels;
    QStringList m_colLabels;
};
