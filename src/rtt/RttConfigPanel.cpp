/**
 * @file RttConfigPanel.cpp
 * @brief RTT 配置面板实现 — 骨架文件
 */

#include "rtt/RttConfigPanel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
RttConfigPanel::RttConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_deviceCombo(nullptr)
    , m_interfaceCombo(nullptr)
    , m_speedSpin(nullptr)
    , m_channelSpin(nullptr)
{
    setObjectName(QStringLiteral("RttConfigPanel"));
    setupUI();
}

/**
 * @brief 获取当前配置参数
 * @return 包含设备/接口/速度/通道等参数的 QVariantMap
 */
QVariantMap RttConfigPanel::config() const
{
    QVariantMap cfg;
    // TODO: 从控件读取当前值填充 cfg
    return cfg;
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 创建设备选择、接口选择、速度、通道号等控件，
 * 并排列为水平/垂直布局。
 */
void RttConfigPanel::setupUI()
{
    // TODO: 创建并布局所有 UI 控件
    // m_deviceCombo = new QComboBox(this);
    // m_interfaceCombo = new QComboBox(this);
    // m_speedSpin = new QSpinBox(this);
    // m_channelSpin = new QSpinBox(this);
}
