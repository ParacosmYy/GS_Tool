#ifndef YMODEMTRANSFER_H
#define YMODEMTRANSFER_H

#include <QObject>
#include <QTimer>
#include "connection/IConnection.h"
#include "utils/CRC.h"

// YMODEM协议传输器 - PC端Sender实现
// 基于XMODEM-CRC，增加Block 0文件信息和批量传输
// 复用CRC16-CCITT校验和IConnection接口
class YModemTransfer : public QObject {
    Q_OBJECT

public:
    explicit YModemTransfer(QObject* parent = nullptr);

    void setConnection(IConnection* conn);
    void setFilePath(const QString& path);
    void setFilePaths(const QStringList& paths);
    bool start();
    void cancel();
    bool isRunning() const;

signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);

private slots:
    void onConnectionReadyRead(const QByteArray& data);
    void onTimeout();

private:
    static constexpr char SOH = 0x01;
    static constexpr char STX = 0x02;
    static constexpr char EOT = 0x04;
    static constexpr char ACK = 0x06;
    static constexpr char NAK = 0x15;
    static constexpr char CAN = 0x18;
    static constexpr char CRC_CHAR = 'C';

    static constexpr int kBlockSize = 128;
    static constexpr int kMaxRetries = 10;
    static constexpr int kTimeoutMs = 5000;

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
    void processReceivedData();
    void sendBlock0();
    void sendBlock();
    void sendEOT();
    void sendFinalBlock0();
    void sendCancel();
    void finishTransfer();
    QByteArray buildBlock(int blockNum, const QByteArray& blockData);
    QByteArray buildBlock0(const QString& fileName, qint64 fileSize);

    IConnection* m_conn = nullptr;
    QStringList m_filePaths;
    QByteArray m_currentData;
    QString m_currentFileName;
    QByteArray m_receiveBuffer;

    State m_state = State::Idle;
    int m_blockNumber = 0;
    qint64 m_bytesSent = 0;
    qint64 m_totalBytes = 0;
    int m_fileIndex = 0;
    int m_retryCount = 0;
    bool m_cancelled = false;

    QTimer* m_timeoutTimer;
};

#endif // YMODEMTRANSFER_H
