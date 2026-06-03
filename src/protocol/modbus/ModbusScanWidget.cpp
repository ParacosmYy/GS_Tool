/**
 * @file ModbusScanWidget.cpp
 * @brief Modbus从站地址扫描器实现
 *
 * 逐地址发送探测请求，收集在线从站列表并更新UI。
 * 提供地址范围输入、进度条和结果列表。
 */
#include "protocol/modbus/ModbusScanWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

/** @brief 构造Modbus从站扫描器(地址范围+进度条+结果列表) @param parent 父控件 */
ModbusScanWidget::ModbusScanWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("ModbusScanWidget");

    auto* layout = new QVBoxLayout(this);

    // 地址范围行
    auto* rangeLayout = new QHBoxLayout();
    rangeLayout->addWidget(new QLabel(tr("起始地址:"), this));

    m_fromSpin = new QSpinBox(this);
    m_fromSpin->setObjectName("fromSpin");
    m_fromSpin->setRange(1, 247);
    m_fromSpin->setValue(1);
    rangeLayout->addWidget(m_fromSpin);

    rangeLayout->addWidget(new QLabel(tr("结束地址:"), this));

    m_toSpin = new QSpinBox(this);
    m_toSpin->setObjectName("toSpin");
    m_toSpin->setRange(1, 247);
    m_toSpin->setValue(247);
    rangeLayout->addWidget(m_toSpin);

    rangeLayout->addStretch();
    layout->addLayout(rangeLayout);

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName("scanProgressBar");
    m_progressBar->setRange(0, 247);
    m_progressBar->setTextVisible(true);
    layout->addWidget(m_progressBar);

    // 扫描按钮
    m_scanBtn = new QPushButton(tr("开始扫描"), this);
    m_scanBtn->setObjectName("scanButton");
    layout->addWidget(m_scanBtn);

    // 结果列表
    m_resultList = new QListWidget(this);
    m_resultList->setObjectName("scanResultList");
    layout->addWidget(m_resultList);

    connect(m_scanBtn, &QPushButton::clicked, this, [this]() {
        if (m_scanning) {
            stopScan();
        } else {
            startScan(m_fromSpin->value(), m_toSpin->value());
        }
    });
}

/** @brief 设置ModbusMaster实例(绑定响应/超时信号) @param master Modbus主站指针 */
void ModbusScanWidget::setModbusMaster(ModbusMaster* master) {
    if (m_master) {
        disconnect(m_master, &ModbusMaster::responseReceived,
                   this, &ModbusScanWidget::onResponseReceived);
        disconnect(m_master, &ModbusMaster::timeout,
                   this, &ModbusScanWidget::onScanTimeout);
    }
    m_master = master;
    if (m_master) {
        connect(m_master, &ModbusMaster::responseReceived,
                this, &ModbusScanWidget::onResponseReceived);
        connect(m_master, &ModbusMaster::timeout,
                this, &ModbusScanWidget::onScanTimeout);
    }
}

/** @brief 开始扫描指定地址范围 @param from 起始地址 @param to 结束地址 */
void ModbusScanWidget::startScan(int from, int to) {
    m_scanFrom    = from;
    m_scanTo      = to;
    m_currentAddr = from;
    m_scanning    = true;
    ++m_totalScansInitiated;

    m_resultList->clear();
    m_progressBar->setRange(from, to);
    m_progressBar->setValue(from);
    updateScanButtonState(true);

    // 禁用范围输入
    m_fromSpin->setEnabled(false);
    m_toSpin->setEnabled(false);

    scanNext();
}

/** @brief 停止正在进行的扫描，恢复UI状态并发射scanCompleted信号 */
void ModbusScanWidget::stopScan() {
    m_scanning = false;
    updateScanButtonState(false);

    // 恢复范围输入
    m_fromSpin->setEnabled(true);
    m_toSpin->setEnabled(true);

    emit scanCompleted();
}

/** @brief 获取已发现的从站地址列表 @return 从站地址列表 */
QList<int> ModbusScanWidget::foundSlaves() const {
    QList<int> slaves;
    for (int i = 0; i < m_resultList->count(); ++i) {
        QListWidgetItem* item = m_resultList->item(i);
        slaves.append(item->data(Qt::UserRole).toInt());
    }
    return slaves;
}

/** @brief ModbusMaster响应到达回调，添加从站到结果列表并继续扫描 @param frame 接收到的Modbus帧 */
void ModbusScanWidget::onResponseReceived(const ModbusFrame& frame) {
    int addr = static_cast<int>(frame.slaveAddress);
    QString desc = tr("从站 %1 (功能码: %2)")
                       .arg(addr)
                       .arg(static_cast<int>(frame.function));
    m_resultList->addItem(desc);
    m_resultList->item(m_resultList->count() - 1)
        ->setData(Qt::UserRole, addr);
    emit slaveFound(addr, desc);

    if (m_scanning) { scanNext(); }
}

/** @brief 超时回调：跳过当前地址，继续扫描下一个 @param slave 超时从站地址 @param function 超时功能码 */
void ModbusScanWidget::onScanTimeout(int slave, int function) {
    Q_UNUSED(slave)
    Q_UNUSED(function)
    if (m_scanning) { scanNext(); }
}

/** @brief 发送下一个地址的探测请求(读1个保持寄存器)并更新进度条 */
void ModbusScanWidget::scanNext() {
    if (m_currentAddr > m_scanTo || !m_scanning) {
        stopScan();
        return;
    }

    m_progressBar->setValue(m_currentAddr);
    emit scanProgress(m_currentAddr);

    // 发送探测请求（读1个保持寄存器）
    if (m_master) {
        m_master->readRegisters(m_currentAddr, 0, 1);
        ++m_totalRegistersRead;
    }
    m_currentAddr++;
}

/** @brief 更新扫描按钮文字(扫描中="停止扫描"/空闲="开始扫描") @param scanning true=正在扫描 */
void ModbusScanWidget::updateScanButtonState(bool scanning) {
    if (scanning) {
        m_scanBtn->setText(tr("停止扫描"));
    } else {
        m_scanBtn->setText(tr("开始扫描"));
    }
    m_scanBtn->setEnabled(true);
}

/** @brief 获取累计发起扫描次数 */
quint64 ModbusScanWidget::totalScansInitiated() const
{
    return m_totalScansInitiated;
}

/** @brief 获取累计读取寄存器次数 */
quint64 ModbusScanWidget::totalRegistersRead() const
{
    return m_totalRegistersRead;
}

/** @brief 重置所有统计计数器 */
void ModbusScanWidget::resetStatistics()
{
    m_totalScansInitiated = 0;
    m_totalRegistersRead = 0;
}
