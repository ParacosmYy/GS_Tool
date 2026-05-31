#ifndef XMODEMTRANSFER_H
#define XMODEMTRANSFER_H

#include <QObject>
#include <QTimer>
#include "connection/IConnection.h"
#include "utils/CRC.h"

// XMODEM协议传输器 - PC端Sender实现
// 支持三种模式: Checksum(Sum8), CRC16, 1K(1024字节块+CRC16)
// 通过IConnection接口发送，不依赖具体连接类型
class XModemTransfer : public QObject {
    Q_OBJECT

public:
    enum Mode {
        Checksum, // XMODEM-Checksum: SOH + 128B + Sum8
        CRC,      // XMODEM-CRC: SOH + 128B + CRC16
        OneK      // XMODEM-1K: STX + 1024B + CRC16
    };

    explicit XModemTransfer(QObject* parent = nullptr);

    // 设置传输连接（串口/TCP/UDP）
    void setConnection(IConnection* conn);

    // 设置XMODEM模式
    void setMode(Mode mode);

    // 设置要传输的文件路径
    void setFilePath(const QString& path);

    // 设置固件数据（直接传入二进制，跳过文件读取）
    void setData(const QByteArray& data);

    // 开始传输，返回是否成功启动
    bool start();

    // 取消传输
    void cancel();

    // 获取当前传输状态
    bool isRunning() const;

signals:
    // 传输进度: percent 0-100, bytesSent, totalBytes
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);

    // 传输完成
    void transferComplete();

    // 传输错误
    void transferError(const QString& reason);

private slots:
    void onConnectionReadyRead(const QByteArray& data);
    void onTimeout();

private:
    // XMODEM协议控制字节
    static constexpr char SOH = 0x01;   // 128字节块头
    static constexpr char STX = 0x02;   // 1024字节块头
    static constexpr char EOT = 0x04;   // 传输结束
    static constexpr char ACK = 0x06;   // 确认
    static constexpr char NAK = 0x15;   // 否定确认(Checksum模式)
    static constexpr char CAN = 0x18;   // 取消传输
    static constexpr char CRC_CHAR = 'C'; // CRC模式请求

    // 传输状态机
    enum class State {
        Idle,
        WaitingForStart,    // 等待接收方发送NAK或'C'
        SendingBlock,       // 发送数据块等待ACK
        SendingEOT,         // 发送EOT等待ACK
        Done,
        Error
    };

    void setState(State newState);
    void processReceivedData();
    void sendBlock();
    void sendEOT();
    void finishTransfer();
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);
    quint16 xmodemCrc(const QByteArray& data);

    IConnection* m_conn = nullptr;
    Mode m_mode = CRC;
    QString m_filePath;
    QByteArray m_data;
    QByteArray m_receiveBuffer;

    State m_state = State::Idle;
    int m_blockNumber = 1;
    qint64 m_bytesSent = 0;
    int m_retryCount = 0;
    bool m_cancelled = false;

    QTimer* m_timeoutTimer;
    static constexpr int kMaxRetries = 10;
    static constexpr int kTimeoutMs = 5000;

    // 块大小由模式决定
    int blockSize() const {
        return (m_mode == OneK) ? 1024 : 128;
    }
};

#endif // XMODEMTRANSFER_H
