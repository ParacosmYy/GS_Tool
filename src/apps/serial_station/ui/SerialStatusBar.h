#ifndef SERIAL_STATUS_BAR_H
#define SERIAL_STATUS_BAR_H

#include <QtWidgets/QWidget>

#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/SerialStationModels.h"

class QLabel;

namespace serial_station {

/**
 * @brief Serial Station 底部状态栏。
 *
 * 只展示会话摘要和计数器，不处理串口状态机。
 */
class SerialStatusBar : public QWidget {
    Q_OBJECT

public:
    explicit SerialStatusBar(QWidget* parent = nullptr);

    /**
     * @brief 同步串口会话状态。
     * @param state 会话状态
     */
    void setSessionState(SerialSessionState state);

    /**
     * @brief 同步当前端口配置摘要。
     * @param config UART 配置
     */
    void setPortConfig(const SerialPortConfig& config);

    /**
     * @brief 增加发送计数。
     */
    void incrementTx();

    /**
     * @brief 增加接收计数。
     */
    void incrementRx();

    /**
     * @brief 增加错误计数。
     */
    void incrementErrors();

    /**
     * @brief 重置所有计数器。
     */
    void resetCounters();

private:
    void setupUi();
    void refreshCounters();
    QString stateText(SerialSessionState state) const;

    QLabel* m_stateLabel = nullptr;
    QLabel* m_portLabel = nullptr;
    QLabel* m_baudLabel = nullptr;
    QLabel* m_txLabel = nullptr;
    QLabel* m_rxLabel = nullptr;
    QLabel* m_errorLabel = nullptr;
    int m_txCount = 0;
    int m_rxCount = 0;
    int m_errorCount = 0;
};

} // namespace serial_station

#endif // SERIAL_STATUS_BAR_H
