/**
 * @file ModbusMasterPanel.h
 * @brief Modbus Master交互面板 -- 请求参数编辑、发送/轮询控制、响应表格显示
 *
 * 左侧: 从站地址、功能码、起始寄存器、数量/值、超时、重试次数。
 * 右侧: 响应数据表格(地址+HEX+DEC)、轮询间隔、启动/停止按钮。
 * 统计接口: @see ModbusMasterPanelStats.cpp
 */
#ifndef MODBUS_MASTER_PANEL_H
#define MODBUS_MASTER_PANEL_H

#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include "protocol/modbus_master/ModbusMaster.h"

/**
 * @brief Modbus Master交互面板
 * 组合ModbusMasterPoller，提供可视化请求构建和响应浏览。
 */
class ModbusMasterPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造面板 @param poller 轮询调度器(外部拥有) @param parent 父控件 */
    explicit ModbusMasterPanel(ModbusMasterPoller* poller, QWidget* parent = nullptr);

    /** @brief 获取累计发送按钮点击次数 */
    quint64 totalSendClicks() const { return m_totalSendClicks; }

    /** @brief 获取累计轮询启停次数 */
    quint64 totalPollToggles() const { return m_totalPollToggles; }

    /** @brief 获取累计响应表格更新次数 */
    quint64 totalResponseUpdates() const { return m_totalResponseUpdates; }

    /** @brief 重置面板统计 */
    void resetPanelStatistics();

private slots:
    void onSendClicked();
    void onPollToggle();
    void onResponseReceived(const ModbusMasterResponse& resp);
    void onRequestSent(const ModbusMasterRequest& req);
    void onErrorOccurred(const QString& msg);

private:
    void setupUI();
    QGroupBox* createConfigGroup();
    QGroupBox* createResponseGroup();
    void updateResponseTable(const ModbusMasterResponse& resp);
    void updateStatusLabel();

    ModbusMasterPoller* m_poller = nullptr;

    // 左侧配置控件
    QSpinBox*   m_slaveSpin    = nullptr; ///< 从站地址
    QComboBox*  m_funcCombo    = nullptr; ///< 功能码
    QSpinBox*   m_startRegSpin = nullptr; ///< 起始寄存器
    QSpinBox*   m_countSpin    = nullptr; ///< 数量
    QSpinBox*   m_valueSpin    = nullptr; ///< 写入值
    QSpinBox*   m_timeoutSpin  = nullptr; ///< 超时
    QSpinBox*   m_retrySpin    = nullptr; ///< 重试次数
    QPushButton* m_sendBtn     = nullptr; ///< 发送按钮

    // 右侧响应控件
    QTableWidget* m_responseTable = nullptr; ///< 响应表格
    QSpinBox*     m_pollIntervalSpin = nullptr; ///< 轮询间隔
    QPushButton*  m_pollBtn      = nullptr; ///< 轮询开关
    QLabel*       m_statusLabel  = nullptr; ///< 状态标签
    QLabel*       m_errorLabel   = nullptr; ///< 错误标签

    // 面板统计
    quint64 m_totalSendClicks     = 0; ///< 发送按钮点击次数
    quint64 m_totalPollToggles    = 0; ///< 轮询启停次数
    quint64 m_totalResponseUpdates = 0; ///< 响应表格更新次数
};

#endif // MODBUS_MASTER_PANEL_H
