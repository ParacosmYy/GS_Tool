#ifndef ZMODEMTRANSFER_H
#define ZMODEMTRANSFER_H

#include <QObject>
#include <QTimer>
#include "connection/IConnection.h"
#include "utils/CRC.h"

// ZMODEM协议传输器 - PC端Sender实现
// 支持连续发送、CRC32校验、1024字节数据帧
class ZModemTransfer : public QObject {
    Q_OBJECT

public:
    explicit ZModemTransfer(QObject* parent = nullptr);

    void setConnection(IConnection* conn);
    void setFilePath(const QString& path);
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
    // ZMODEM帧类型
    static constexpr quint8 ZRQINIT = 0;    // 请求初始化
    static constexpr quint8 ZRINIT  = 1;    // 接收方初始化
    static constexpr quint8 ZSINIT  = 2;    // 发送方初始化
    static constexpr quint8 ZACK    = 3;    // 确认
    static constexpr quint8 ZFILE   = 4;    // 文件信息
    static constexpr quint8 ZSKIP   = 5;    // 跳过文件
    static constexpr quint8 ZNAK    = 6;    // 否定确认
    static constexpr quint8 ZABORT  = 7;    // 中止
    static constexpr quint8 ZFIN    = 8;    // 结束会话
    static constexpr quint8 ZRPOS   = 9;    // 重传位置
    static constexpr quint8 ZDATA   = 10;   // 数据帧
    static constexpr quint8 ZEOF    = 11;   // 文件结束
    static constexpr quint8 ZFERR   = 12;   // 错误
    static constexpr quint8 ZCRC    = 13;   // 请求CRC
    static constexpr quint8 ZCHALLENGE = 14;
    static constexpr quint8 ZCOMPL  = 15;

    // ZMODEM帧头标志
    static constexpr char ZPAD   = '*';     // 0x2A
    static constexpr char ZDLE   = 0x18;    // 0x18
    static constexpr char ZBIN   = 'A';     // 二进制帧(16bit CRC)
    static constexpr char ZBIN32 = 'B';     // 二进制帧(32bit CRC)
    static constexpr char ZHEX   = 'C';     // 十六进制帧

    // 数据子帧结束标记
    static constexpr char ZCRCE  = 'h';     // CRC下一帧
    static constexpr char ZCRCG  = 'i';     // CRC继续
    static constexpr char ZCRCQ  = 'j';     // CRC+ZACK
    static constexpr char ZCRCW  = 'k';     // CRC+ZACK等待

    static constexpr int kDataLen = 1024;
    static constexpr int kMaxRetries = 10;
    static constexpr int kTimeoutMs = 10000;

    enum class State {
        Idle,
        WaitingRinit,
        SendingFile,
        SendingData,
        WaitingZAck,
        SendingEof,
        SendingFin,
        Done,
        Error
    };

    void setState(State s);
    void processReceivedData();
    bool parseHexFrame(const QByteArray& data, int& type, QByteArray& headerData);
    QByteArray buildHexHeader(quint8 type, const QByteArray& data = QByteArray());
    QByteArray buildBinHeader(quint8 type, const QByteArray& data = QByteArray());
    QByteArray buildDataSubpacket(char endFlag, const QByteArray& data);
    void sendDataSubpackets();
    void sendZRQINIT();
    void sendZFILE();
    void sendZDATA();
    void sendZEOF();
    void sendZFIN();
    void sendCancel();
    void finishTransfer();
    quint32 encodeCrc32(quint32 crc);
    QByteArray toHex(quint32 val, int digits);

    IConnection* m_conn = nullptr;
    QString m_filePath;
    QByteArray m_fileData;
    QByteArray m_receiveBuffer;

    State m_state = State::Idle;
    qint64 m_bytesSent = 0;
    qint64 m_fileOffset = 0;
    int m_retryCount = 0;
    bool m_cancelled = false;
    quint32 m_senderCrc32 = 0;

    QTimer* m_timeoutTimer;
};

#endif // ZMODEMTRANSFER_H
