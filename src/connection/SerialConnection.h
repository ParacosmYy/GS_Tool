/**
 * @file SerialConnection.h
 * @brief 串口连接实现 - 封装 QSerialPort，提供完整的串口通信能力
 *
 * 职责:
 *   1. 封装 QSerialPort 的打开/关闭/读写操作
 *   2. 提供串口参数配置（端口名/波特率/数据位/校验/停止位/流控/DTR/RTS）
 *   3. 错误检测与分类（端口不存在/被占用/权限不足/意外断开）
 *   4. 将 QSerialPort 的信号翻译为 IConnection 的统一信号
 *
 * 设计模式:
 *   - 适配器模式: 将 QSerialPort 适配到 IConnection 接口
 *
 * 协作关系:
 *   - IConnection: 父接口，定义统一的连接抽象
 *   - ConnectionController: 上层控制器，通过 IConnection 指针管理本类
 *   - ConnectionFactory: 工厂创建本类实例
 */

#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include "IConnection.h"
#include <QSerialPort>
#include <QSerialPortInfo>

/**
 * @brief 串口连接实现 - 封装 QSerialPort
 *
 * 通过 IConnection 接口暴露串口能力，上层模块不需要直接接触 QSerialPort。
 * 所有串口特有的错误都会被翻译成人类可读的中文描述后通过 errorOccurred 信号发出。
 */
class SerialConnection : public IConnection {
    Q_OBJECT

public:
    /**
     * @brief 构造串口连接
     * @param parent 父对象（通常为 nullptr，由 ConnectionManager 管理生命周期）
     */
    explicit SerialConnection(QObject* parent = nullptr);

    /** @brief 析构函数，自动关闭串口 */
    ~SerialConnection() override;

    // ---- IConnection 接口实现 ----

    /** @brief 返回连接类型为 Serial */
    ConnectionType type() const override { return ConnectionType::Serial; }

    /** @brief 返回端口名称（如 "COM3"） */
    QString name() const override;

    /** @brief 返回当前连接状态 */
    ConnectionState state() const override;

    /**
     * @brief 打开串口连接
     *
     * 失败时会根据错误类型发出详细的中文错误描述:
     *   - 端口名为空
     *   - 端口不存在
     *   - 端口被其他程序占用
     *   - 权限不足
     *   - 其他系统错误
     *
     * @return true=打开成功, false=打开失败
     */
    bool open() override;

    /** @brief 关闭串口连接并重置状态为 Disconnected */
    void close() override;

    /**
     * @brief 写入数据到串口
     * @param data 待发送的原始字节数据
     * @return 实际写入的字节数，-1表示失败
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 通过 QVariantMap 配置串口参数
     *
     * 支持的 key:
     *   - "portName" (string): 端口名，如 "COM3"
     *   - "baudRate" (int): 波特率，如 115200
     *   - "dataBits" (int): 数据位，范围 5-8
     *   - "parity" (int): 校验，0=无 1=偶 2=奇 3=Mark 4=Space
     *   - "stopBits" (int): 停止位，0=1 1=1.5 2=2
     *   - "flowControl" (int): 流控，0=无 1=RTS/CTS 2=XON/XOFF
     *   - "dtr" (bool): DTR 信号
     *   - "rts" (bool): RTS 信号
     *
     * 未识别的 key 会被安全忽略。
     * @param params 参数映射表
     */
    void configure(const QVariantMap& params) override;

    // ---- 串口参数配置接口 ----

    /** @brief 设置端口名（如 "COM3"），仅在未打开时生效 */
    void setPortName(const QString& portName);

    /** @brief 获取当前端口名 */
    QString portName() const;

    /** @brief 设置波特率（如 115200） */
    void setBaudRate(qint32 baud);

    /** @brief 获取当前波特率 */
    qint32 baudRate() const;

    /** @brief 设置数据位（5/6/7/8） */
    void setDataBits(QSerialPort::DataBits bits);

    /** @brief 获取当前数据位 */
    QSerialPort::DataBits dataBits() const;

    /** @brief 设置校验模式 */
    void setParity(QSerialPort::Parity parity);

    /** @brief 获取当前校验模式 */
    QSerialPort::Parity parity() const;

    /** @brief 设置停止位 */
    void setStopBits(QSerialPort::StopBits bits);

    /** @brief 获取当前停止位 */
    QSerialPort::StopBits stopBits() const;

    /** @brief 设置流控模式 */
    void setFlowControl(QSerialPort::FlowControl control);

    /** @brief 获取当前流控模式 */
    QSerialPort::FlowControl flowControl() const;

    /** @brief 设置 DTR 信号电平（重写 IConnection 虚方法） */
    void setDtr(bool enabled) override;

    /** @brief 设置 RTS 信号电平（重写 IConnection 虚方法） */
    void setRts(bool enabled) override;

    /** @brief 查询 DTR 信号当前状态（重写 IConnection 虚方法） */
    bool isDtr() const override;

    /** @brief 查询 RTS 信号当前状态（重写 IConnection 虚方法） */
    bool isRts() const override;

    /**
     * @brief 获取系统中所有可用的串口列表
     * @return QSerialPortInfo 列表，包含端口名、描述、制造商等信息
     */
    static QList<QSerialPortInfo> availablePorts();

private slots:
    /** @brief QSerialPort::readyRead 信号处理，读取所有可用数据并转发 */
    void onReadyRead();

    /**
     * @brief QSerialPort::errorOccurred 信号处理
     *
     * 将 QSerialPort 的系统错误码翻译为人类可读的中文描述，
     * 更新内部状态为 Error，并通过 IConnection::errorOccurred 信号通知上层。
     * @param error QSerialPort 的错误码
     */
    void onError(QSerialPort::SerialPortError error);

private:
    /**
     * @brief 将 QSerialPort 错误码翻译为详细的中文错误描述
     *
     * 根据错误类型结合当前上下文给出具体的诊断信息，帮助用户快速定位问题:
     *   - 端口不存在 → 提示检查设备连接
     *   - 被占用 → 提示关闭占用程序
     *   - 权限不足 → 提示以管理员身份运行
     *   - 意外断开 → 提示设备可能被拔出
     *
     * @param error QSerialPort 错误码
     * @return 人类可读的中文错误描述
     */
    QString translateError(QSerialPort::SerialPortError error);

    /** @brief Qt 串口对象，提供底层串口操作能力 */
    QSerialPort m_serial;

    /** @brief 端口名称，如 "COM3" */
    QString m_portName;

    /** @brief 当前连接状态 */
    ConnectionState m_state = ConnectionState::Disconnected;
};

#endif // SERIALCONNECTION_H
