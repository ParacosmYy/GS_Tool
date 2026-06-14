#ifndef SERIAL_COMMAND_PANEL_H
#define SERIAL_COMMAND_PANEL_H

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QDateTime>
#include <QtWidgets/QWidget>

#include "apps/serial_station/ui/SerialCommandHistoryModel.h"

class QComboBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QCheckBox;
class QToolButton;

namespace serial_station {

struct SerialProfileCommand;

/**
 * @brief Serial Station 命令发送面板。
 *
 * 只收集用户输入和发送意图，不拼接协议帧，不直接访问串口 core。
 */
class SerialCommandPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialCommandPanel(QWidget* parent = nullptr);

    /**
     * @brief 当前命令输入内容。
     * @return 去除首尾空白后的命令文本
     */
    QString commandText() const;

    /**
     * @brief 当前发送模式。
     * @return UI 中选择的模式标识
     */
    QString sendMode() const;

    /**
     * @brief 设置发送按钮可用性。
     * @param enabled true 表示允许发送
     */
    void setSendEnabled(bool enabled);

    /**
     * @brief 最近命令数量。
     */
    int historyCount() const;

    /**
     * @brief 最近命令文本快照。
     */
    QStringList historyCommands() const;

    /**
     * @brief 将档案中的默认发送模式和命令列表应用到 UI。
     */
    void applyProfileCommands(const QVector<SerialProfileCommand>& commands,
                              const QString& defaultMode);

public slots:
    /**
     * @brief 确认最近一次发送成功并写入历史。
     */
    void confirmLastSentCommand();

    /**
     * @brief 记录一条已经发送成功的命令。
     * @param command 命令文本
     * @param mode 发送模式
     */
    void recordSentCommand(const QString& command, const QString& mode);
    /**
     * @brief 显示命令发送失败信息（用于错误反馈）
     * @param message 失败原因
     */
    void notifyCommandFailed(const QString& message);
    /**
     * @brief 显示带失败分类的命令发送失败信息（用于错误归类）
     * @param reason 失败分类码
     * @param message 失败原因
     */
    void notifyCommandFailed(const QString& reason, const QString& message);

signals:
    /**
     * @brief 用户请求发送一条命令。
     * @param command 命令文本
     * @param mode 发送模式
     * @param retryCount 重试次数
     * @param retryDelayMs 每次重试间隔（毫秒）
     */
    void sendRequested(const QString& command,
                       const QString& mode,
                       int retryCount,
                       int retryDelayMs);

    /**
     * @brief 用户选择了一个快捷命令。
     * @param command 快捷命令文本
     */
    void quickCommandSelected(const QString& command);

private slots:
    void emitSendRequested();
    void applyQuickCommand();
    void applyHistoryCommand(int index);
    void clearHistory();
    void updateSendButtonState();
    void refreshStatus(const QString& status, const QString& state);

private:
    void setupUi();
    void connectSignals();
    QToolButton* createQuickButton(const QString& text, const QString& command);
    void refreshHistoryUi();
    void setModeById(const QString& mode);
    QString failureReasonLabel(const QString& reason) const;
    int retryCount() const;
    int retryDelayMs() const;

    QLineEdit* m_commandEdit = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QComboBox* m_historyCombo = nullptr;
    QLabel* m_statusLabel = nullptr;
    QCheckBox* m_appendLineBreakCheck = nullptr;
    QPushButton* m_sendButton = nullptr;
    QPushButton* m_clearHistoryButton = nullptr;
    QSpinBox* m_retryCountSpin = nullptr;
    QSpinBox* m_retryIntervalSpin = nullptr;
    QToolButton* m_readIdButton = nullptr;
    QToolButton* m_pingButton = nullptr;
    QToolButton* m_resetButton = nullptr;
    SerialCommandHistoryModel m_history;
    QString m_pendingCommand;
    QString m_pendingRawCommand;
    QString m_pendingMode;
    QDateTime m_lastSendTime;
    constexpr static int kSendIntervalMs = 180;
};

} // namespace serial_station

#endif // SERIAL_COMMAND_PANEL_H
