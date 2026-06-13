#ifndef SERIAL_PORT_PANEL_H
#define SERIAL_PORT_PANEL_H

#include <QtWidgets/QWidget>

#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/SerialStationModels.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

namespace serial_station {

/**
 * @brief Serial Station 的 UART 配置面板。
 *
 * 该面板只展示和收集串口参数，通过 signal 把用户意图交给
 * SerialStationController，不直接调用 SerialManager 或打开串口。
 */
class SerialPortPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialPortPanel(QWidget* parent = nullptr);

    /**
     * @brief 返回当前 UI 中选择的 UART 配置。
     * @return 串口配置值对象
     */
    SerialPortConfig currentConfig() const;

    /**
     * @brief 将档案中的 UART 配置应用到 UI 控件。
     * @param config UART 配置值对象
     */
    void applyConfig(const SerialPortConfig& config);

    /**
     * @brief 把会话状态同步到按钮和状态文案。
     * @param state 当前串口会话状态
     */
    void setSessionState(SerialSessionState state);

    /**
     * @brief 显示最近一次错误。
     * @param message 错误描述
     */
    void setErrorMessage(const QString& message);

signals:
    /**
     * @brief 用户请求打开串口。
     * @param config 当前 UART 配置
     */
    void connectRequested(const SerialPortConfig& config);

    /**
     * @brief 用户请求关闭串口。
     */
    void disconnectRequested();

    /**
     * @brief 用户刷新端口列表。
     */
    void refreshRequested();

private slots:
    void refreshPorts();
    void emitConnectRequested();
    void updateSummary();

private:
    void setupUi();
    void setupCombos();
    void connectSignals();
    void setStatusText(const QString& text, const QString& stateName);

    QComboBox* m_portCombo = nullptr;
    QComboBox* m_baudCombo = nullptr;
    QComboBox* m_dataBitsCombo = nullptr;
    QComboBox* m_parityCombo = nullptr;
    QComboBox* m_stopBitsCombo = nullptr;
    QComboBox* m_flowControlCombo = nullptr;
    QCheckBox* m_dtrCheck = nullptr;
    QCheckBox* m_rtsCheck = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_summaryLabel = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QPushButton* m_connectButton = nullptr;
    QPushButton* m_disconnectButton = nullptr;
};

} // namespace serial_station

#endif // SERIAL_PORT_PANEL_H
