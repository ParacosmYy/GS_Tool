#ifndef YMODEMTRANSFER_H
#define YMODEMTRANSFER_H

#include "ota/protocols/BaseTransfer.h"
#include "utils/CRC.h"

// YMODEM协议传输器 - PC端Sender实现
// 基于XMODEM-CRC，增加Block 0文件信息和批量传输
// 继承BaseTransfer，通过4个纯虚钩子注入协议特有逻辑
class YModemTransfer : public BaseTransfer {
    Q_OBJECT

public:
    explicit YModemTransfer(QObject* parent = nullptr);

    void setFilePath(const QString& path);
    void setFilePaths(const QStringList& paths);

protected:
    // === BaseTransfer 钩子实现 ===
    bool onStartInit() override;
    void sendCancelBytes() override;
    void processReceivedData() override;
    void handleTimeout() override;

private:
    static constexpr char SOH = 0x01;
    static constexpr char EOT = 0x04;
    static constexpr char ACK = 0x06;
    static constexpr char NAK = 0x15;
    static constexpr char CAN = 0x18;
    static constexpr char CRC_CHAR = 'C';

    static constexpr int kBlockSize = 128;

    // YMODEM内部状态(独立于BaseTransfer的TransferState)
    enum class State {
        Idle,
        WaitingStart,
        SendingBlock0,
        SendingData,
        SendingEOT,
        WaitBlock0Ack,
        WaitFinalC,
        SendingFinalBlock0,
        Done,
        Error
    };

    void setState(State s);
    void sendBlock0();
    void sendBlock();
    void sendEOT();
    void sendFinalBlock0();
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);
    QByteArray buildBlock0(const QString& fileName, qint64 fileSize);

    QStringList m_filePaths;
    QByteArray m_currentData;
    QString m_currentFileName;

    State m_ymodemState = State::Idle;
    int m_blockNumber = 0;
    qint64 m_bytesSent = 0;
    qint64 m_totalBytes = 0;
    int m_fileIndex = 0;
};

#endif // YMODEMTRANSFER_H
