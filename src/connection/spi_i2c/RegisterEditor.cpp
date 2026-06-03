/**
 * @file RegisterEditor.cpp
 * @brief 寄存器编辑器实现
 */

#include "connection/spi_i2c/RegisterEditor.h"
#include "connection/interface/IConnection.h"
#include "connection/spi_i2c/I2cConnection.h"
#include "connection/spi_i2c/SpiConnection.h"
#include <QHBoxLayout>

/**
 * @brief 构造函数 - 初始化UI
 * @param parent 父控件
 */
RegisterEditor::RegisterEditor(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("RegisterEditor");
    setupUi();
    setupConnections();
}

/**
 * @brief 设置底层连接
 * @param connection IConnection实例
 */
void RegisterEditor::setConnection(IConnection* connection)
{
    m_connection = connection;
}

/**
 * @brief 设置默认读取长度
 * @param length 读取字节数
 */
void RegisterEditor::setReadLength(int length)
{
    m_readLength = qMax(1, length);
    if (m_lengthSpin) {
        m_lengthSpin->setValue(m_readLength);
    }
}

/**
 * @brief 读取指定地址的寄存器
 * @param address 寄存器地址
 *
 * 根据连接类型调用I2C或SPI的读方法。
 * 日志格式: "R ADDR: 0x42 → DATA: 0x1A 0x2B"
 */
void RegisterEditor::readAddress(int address)
{
    if (!m_connection) {
        appendLog(tr("R [0x%1] → 错误: 未连接").arg(address, 2, 16, QChar('0')), false);
        return;
    }

    int len = m_lengthSpin ? m_lengthSpin->value() : m_readLength;
    QByteArray data;

    /// 根据连接类型分派读操作
    auto* i2c = qobject_cast<I2cConnection*>(m_connection);
    if (i2c) {
        data = i2c->readRegister(m_deviceAddress, address, len);
    } else {
        /// SPI或其他类型: 使用通用write/read方式
        QByteArray cmd;
        cmd.append(static_cast<char>(address & 0xFF));
        m_connection->write(cmd);
    }

    /// 记录日志
    QString addrStr = QString("0x%1").arg(address, 2, 16, QChar('0')).toUpper();
    if (data.isEmpty()) {
        appendLog(tr("R 地址: %1 -> (无数据)").arg(addrStr), false);
    } else {
        appendLog(tr("R 地址: %1 -> 数据: %2")
            .arg(addrStr)
            .arg(formatHex(data)), false);
    }

    ++m_readCount;
    ++m_totalRegisterReads;
    emit registerReadComplete(address, data);
}

/**
 * @brief 向指定地址写入数据
 * @param address 寄存器地址
 * @param data 待写入数据
 *
 * 日志格式: "W ADDR: 0x42 ← DATA: 0x1A 0x2B"
 */
void RegisterEditor::writeAddress(int address, const QByteArray& data)
{
    if (!m_connection) {
        appendLog(tr("W [0x%1] ← 错误: 未连接").arg(address, 2, 16, QChar('0')), true);
        return;
    }

    bool success = false;

    /// 根据连接类型分派写操作
    auto* i2c = qobject_cast<I2cConnection*>(m_connection);
    if (i2c) {
        success = i2c->writeRegister(0x00, address, data);
    } else {
        QByteArray cmd;
        cmd.append(static_cast<char>(address & 0xFF));
        cmd.append(data);
        qint64 written = m_connection->write(cmd);
        success = (written > 0);
    }

    QString addrStr = QString("0x%1").arg(address, 2, 16, QChar('0')).toUpper();
    if (success) {
        appendLog(tr("W 地址: %1 <- 数据: %2 [成功]")
            .arg(addrStr)
            .arg(formatHex(data)), true);
    } else {
        appendLog(tr("W 地址: %1 <- 数据: %2 [失败]")
            .arg(addrStr)
            .arg(formatHex(data)), true);
    }

    emit registerWriteComplete(address, success);
    ++m_writeCount;
    ++m_totalRegisterWrites;
}

/**
 * @brief 读取按钮点击
 */
void RegisterEditor::onReadClicked()
{
    if (!m_addrSpin) return;
    readAddress(m_addrSpin->value());
}

/**
 * @brief 写入按钮点击
 */
void RegisterEditor::onWriteClicked()
{
    if (!m_addrSpin || !m_dataEdit) return;
    int addr = m_addrSpin->value();
    QByteArray data = QByteArray::fromHex(m_dataEdit->text().toUtf8());
    writeAddress(addr, data);
}

/**
 * @brief 清空日志按钮点击
 */
void RegisterEditor::onClearLogClicked()
{
    if (m_log) {
        m_log->clear();
    }
}

/**
 * @brief 初始化UI布局
 */
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

/**
 * @brief 初始化信号连接
 */
void RegisterEditor::setupConnections()
{
    connect(m_readBtn, &QPushButton::clicked,
            this, &RegisterEditor::onReadClicked);
    connect(m_writeBtn, &QPushButton::clicked,
            this, &RegisterEditor::onWriteClicked);
    connect(m_clearLogBtn, &QPushButton::clicked,
            this, &RegisterEditor::onClearLogClicked);
}

/**
 * @brief 追加日志
 * @param msg 日志消息
 * @param isTx true=发送(蓝色)，false=接收(绿色)
 */
void RegisterEditor::appendLog(const QString& msg, bool isTx)
{
    if (!m_log) return;
    /// TX用Link色，RX用ToolTipText色，跟随QSS主题
    QString color = isTx ? palette().color(QPalette::Link).name()
                         : palette().color(QPalette::ToolTipText).name();
    m_log->append(QString("<span style='color:%1'>%2</span>").arg(color, msg));
}

/**
 * @brief 格式化字节数组为十六进制字符串
 * @param data 字节数组
 * @return "0x1A 0x2B" 格式字符串
 */
QString RegisterEditor::formatHex(const QByteArray& data)
{
    QStringList hexParts;
    for (char byte : data) {
        hexParts.append(QString("0x%1").arg(
            static_cast<quint8>(byte), 2, 16, QChar('0')).toUpper());
    }
    return hexParts.join(" ");
}

/**
 * @brief 获取读操作次数
 * @return 累计读操作计数
 */
int RegisterEditor::readCount() const
{
    return m_readCount;
}

/**
 * @brief 获取写操作次数
 * @return 累计写操作计数
 */
int RegisterEditor::writeCount() const
{
    return m_writeCount;
}

/**
 * @brief 导出操作日志为文本
 * @return 日志文本内容
 */
QString RegisterEditor::exportLog() const
{
    if (!m_log) return QString();
    return m_log->toPlainText();
}

/** @brief 获取累计寄存器读取次数(quint64) */
quint64 RegisterEditor::totalRegisterReads() const
{
    return m_totalRegisterReads;
}

/** @brief 获取累计寄存器写入次数(quint64) */
quint64 RegisterEditor::totalRegisterWrites() const
{
    return m_totalRegisterWrites;
}

/** @brief 重置所有统计计数器 */
void RegisterEditor::resetStatistics()
{
    m_totalRegisterReads = 0;
    m_totalRegisterWrites = 0;
}
