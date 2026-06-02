/**
 * @file RegisterEditor.cpp
 * @brief 寄存器编辑器实现 - 骨架
 */

#include "connection/spi_i2c/RegisterEditor.h"
#include "connection/interface/IConnection.h"

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
 * @brief 读取指定地址的寄存器
 * @param address 寄存器地址
 */
void RegisterEditor::readAddress(int address)
{
    Q_UNUSED(address)
    // TODO: 通过m_connection执行寄存器读取
    appendLog(QString("R [0x%1] -> ...").arg(address, 2, 16, QChar('0')), false);
}

/**
 * @brief 向指定地址写入数据
 * @param address 寄存器地址
 * @param data 待写入数据
 */
void RegisterEditor::writeAddress(int address, const QByteArray& data)
{
    Q_UNUSED(address)
    Q_UNUSED(data)
    // TODO: 通过m_connection执行寄存器写入
    appendLog(QString("W [0x%1] <- %2")
        .arg(address, 2, 16, QChar('0'))
        .arg(QString(data.toHex(' ')).toUpper()), true);
}

/**
 * @brief 读取按钮点击
 */
void RegisterEditor::onReadClicked()
{
    if (!m_addrSpin || !m_dataEdit) return;
    int addr = m_addrSpin->value();
    readAddress(addr);
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
 * @brief 初始化UI布局
 */
void RegisterEditor::setupUi()
{
    auto* layout = new QVBoxLayout(this);

    // 地址输入
    m_addrSpin = new QSpinBox(this);
    m_addrSpin->setObjectName("addrSpin");
    m_addrSpin->setRange(0x00, 0xFF);
    m_addrSpin->setDisplayIntegerBase(16);
    m_addrSpin->setPrefix("0x");

    // 数据输入
    m_dataEdit = new QLineEdit(this);
    m_dataEdit->setObjectName("dataEdit");
    m_dataEdit->setPlaceholderText(tr("十六进制数据 (如: AA BB CC)"));

    // 按钮行
    auto* btnLayout = new QHBoxLayout();
    m_readBtn = new QPushButton(tr("读取"), this);
    m_readBtn->setObjectName("readBtn");

    m_writeBtn = new QPushButton(tr("写入"), this);
    m_writeBtn->setObjectName("writeBtn");

    btnLayout->addWidget(m_readBtn);
    btnLayout->addWidget(m_writeBtn);

    // 日志
    m_log = new QTextEdit(this);
    m_log->setObjectName("regLog");
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(150);

    layout->addWidget(m_addrSpin);
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
}

/**
 * @brief 追加日志
 * @param msg 日志消息
 * @param isTx true=发送(蓝色)，false=接收(绿色)
 */
void RegisterEditor::appendLog(const QString& msg, bool isTx)
{
    if (!m_log) return;
    QString color = isTx ? "#4FC3F7" : "#81C784";
    m_log->append(QString("<span style='color:%1'>%2</span>").arg(color, msg));
}
