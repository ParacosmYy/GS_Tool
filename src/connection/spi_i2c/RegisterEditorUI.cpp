/**
 * @file RegisterEditorUI.cpp
 * @brief 寄存器编辑器 — UI布局与信号连接实现
 *
 * 从 RegisterEditor.cpp 拆分而来，包含:
 *   - setupUi(): 初始化地址/长度/数据输入、读写按钮、操作日志的UI布局
 *   - setupConnections(): 按钮信号连接
 *
 * 核心读写方法和日志格式化见 RegisterEditor.cpp。
 * 统计getter/resetStatistics见 RegisterEditorStats.cpp。
 */

#include "connection/spi_i2c/RegisterEditor.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

/** @brief 初始化UI布局: 地址/长度输入行、数据输入、读/写/清空按钮、操作日志 */
void RegisterEditor::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    /// 地址和长度输入行
    auto* addrLayout = new QHBoxLayout();

    auto* addrLabel = new QLabel(tr("地址:"), this);
    addrLabel->setObjectName("regAddrLabel");

    m_addrSpin = new QSpinBox(this);
    m_addrSpin->setObjectName("regAddrSpin");
    m_addrSpin->setRange(0x00, 0xFF);
    m_addrSpin->setDisplayIntegerBase(16);
    m_addrSpin->setPrefix("0x");

    auto* lenLabel = new QLabel(tr("长度:"), this);
    lenLabel->setObjectName("regLenLabel");

    m_lengthSpin = new QSpinBox(this);
    m_lengthSpin->setObjectName("regLengthSpin");
    m_lengthSpin->setRange(1, 256);
    m_lengthSpin->setValue(m_readLength);

    addrLayout->addWidget(addrLabel);
    addrLayout->addWidget(m_addrSpin);
    addrLayout->addWidget(lenLabel);
    addrLayout->addWidget(m_lengthSpin);

    /// 数据输入
    m_dataEdit = new QLineEdit(this);
    m_dataEdit->setObjectName("regDataEdit");
    m_dataEdit->setPlaceholderText(tr("十六进制数据 (如: AA BB CC)"));

    /// 按钮行
    auto* btnLayout = new QHBoxLayout();
    m_readBtn = new QPushButton(tr("读取"), this);
    m_readBtn->setObjectName("regReadBtn");

    m_writeBtn = new QPushButton(tr("写入"), this);
    m_writeBtn->setObjectName("regWriteBtn");

    m_clearLogBtn = new QPushButton(tr("清空日志"), this);
    m_clearLogBtn->setObjectName("regClearLogBtn");

    btnLayout->addWidget(m_readBtn);
    btnLayout->addWidget(m_writeBtn);
    btnLayout->addWidget(m_clearLogBtn);

    /// 日志
    m_log = new QTextEdit(this);
    m_log->setObjectName("regLogText");
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(150);

    layout->addLayout(addrLayout);
    layout->addWidget(m_dataEdit);
    layout->addLayout(btnLayout);
    layout->addWidget(m_log);
}

/** @brief 初始化信号连接: 读/写/清空日志按钮 */
void RegisterEditor::setupConnections()
{
    connect(m_readBtn, &QPushButton::clicked,
            this, &RegisterEditor::onReadClicked);
    connect(m_writeBtn, &QPushButton::clicked,
            this, &RegisterEditor::onWriteClicked);
    connect(m_clearLogBtn, &QPushButton::clicked,
            this, &RegisterEditor::onClearLogClicked);
}
