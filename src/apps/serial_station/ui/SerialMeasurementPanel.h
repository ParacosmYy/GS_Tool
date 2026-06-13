#ifndef SERIAL_MEASUREMENT_PANEL_H
#define SERIAL_MEASUREMENT_PANEL_H

#include <QtCore/QStringList>
#include <QtWidgets/QWidget>

class QLabel;
class QPlainTextEdit;

namespace serial_station {

/**
 * @brief 测量通道摘要面板。
 *
 * 只展示 controller 提供的通道摘要文本，不解析协议帧或串口字节。
 */
class SerialMeasurementPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialMeasurementPanel(QWidget* parent = nullptr);

public slots:
    void setSummaryLines(const QStringList& lines);
    void setTrendLines(const QStringList& lines);
    void clear();

private:
    void setupUi();
    void showEmptyState();

    QLabel* m_summaryLabel = nullptr;
    QPlainTextEdit* m_view = nullptr;
    QLabel* m_trendLabel = nullptr;
    QPlainTextEdit* m_trendView = nullptr;
};

} // namespace serial_station

#endif // SERIAL_MEASUREMENT_PANEL_H
