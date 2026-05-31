#ifndef XMODEMTRANSFER_H
#define XMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

// XMODEM协议传输器 - PC端Sender实现
// 支持三种模式: Checksum(Sum8), CRC16, 1K(1024字节块+CRC16)
// 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑
class XModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    enum Mode {
        Checksum, // XMODEM-Checksum: SOH + 128B + Sum8
        CRC,      // XMODEM-CRC: SOH + 128B + CRC16
        OneK      // XMODEM-1K: STX + 1024B + CRC16
    };

    explicit XModemTransfer(QObject* parent = nullptr);

    void setMode(Mode mode);
    void setFilePath(const QString& path);
    void setData(const QByteArray& data);

protected:
    // === BaseTransfer 钩子实现 ===
    bool onStartInit() override;
    void sendCancelBytes() override;
    void processReceivedData() override;
    void handleTimeout() override;

private:
    // XMODEM协议控制字节
    static constexpr char SOH = 0x01;   // 128字节块头
    static constexpr char STX = 0x02;   // 1024字节块头
    static constexpr char EOT = 0x04;   // 传输结束
    static constexpr char ACK = 0x06;   // 确认
    static constexpr char NAK = 0x15;   // 否定确认(Checksum模式)
    static constexpr char CAN = 0x18;   // 取消传输
    static constexpr char CRC_CHAR = 'C'; // CRC模式请求

    // XMODEM内部状态(独立于BaseTransfer的TransferState)
    enum class State {
        Idle,
        WaitingForStart,    // 等待接收方发送NAK或'C'
        SendingBlock,       // 发送数据块等待ACK
        SendingEOT,         // 发送EOT等待ACK
        Done,
        Error
    };

    void setState(State newState);
    void sendBlock();
    void sendEOT();
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);
    quint16 xmodemCrc(const QByteArray& data);

    Mode m_mode = CRC;
    QString m_filePath;
    QByteArray m_data;

    State m_xmodemState = State::Idle;
    int m_blockNumber = 1;
    qint64 m_bytesSent = 0;

    int blockSize() const {
        return (m_mode == OneK) ? 1024 : 128;
    }
};

#endif // XMODEMTRANSFER_H
