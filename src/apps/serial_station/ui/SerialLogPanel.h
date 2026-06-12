#ifndef SERIAL_LOG_PANEL_H
#define SERIAL_LOG_PANEL_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;

namespace serial_station {

/**
 * @brief Serial Station 收发日志面板。
 *
 * 当前只维护 UI 内存日志，用于工作台预览；文件导出后续交给 services。
 */
class SerialLogPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialLogPanel(QWidget* parent = nullptr);

    /**
     * @brief 添加一条发送方向日志。
     * @param text 日志文本
     */
    void appendTx(const QString& text);

    /**
     * @brief 添加一条接收方向日志。
     * @param text 日志文本
     */
    void appendRx(const QString& text);

    /**
     * @brief 添加一条系统日志。
     * @param text 日志文本
     */
    void appendSystem(const QString& text);

    /**
     * @brief 当前可见日志文本。
     * @return 日志编辑器中的纯文本
     */
    QString plainText() const;

signals:
    /**
     * @brief 用户请求导出日志。
     *
     * 仅表达意图，实际文件写入必须由 service 层实现。
     */
    void exportRequested();

    /**
     * @brief 日志内容被清空。
     */
    void cleared();

private slots:
    void clearLog();
    void updateFilter(const QString& text);

private:
    void setupUi();
    void connectSignals();
    void appendLine(const QString& channel, const QString& text);
    QString timestampText() const;

    QPlainTextEdit* m_logView = nullptr;
    QLineEdit* m_filterEdit = nullptr;
    QLabel* m_countLabel = nullptr;
    QPushButton* m_clearButton = nullptr;
    QPushButton* m_exportButton = nullptr;
    QStringList m_lines;
};

} // namespace serial_station

#endif // SERIAL_LOG_PANEL_H
