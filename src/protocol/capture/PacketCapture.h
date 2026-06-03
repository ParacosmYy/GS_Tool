#pragma once
#include <QObject>
#include <QByteArray>
#include <QList>
#include <QFile>

class PacketCapture : public QObject {
    Q_OBJECT
public:
    struct Packet { QByteArray data; qint64 timestamp; bool isIncoming; int channel; };
    explicit PacketCapture(QObject *parent = nullptr);
    ~PacketCapture() override;
    void startCapture(int maxPackets = 10000);
    void stopCapture();
    bool isCapturing() const;
    void addPacket(const QByteArray &data, bool incoming, int channel = 0);
    QList<Packet> packets() const;
    QList<Packet> packetsByChannel(int ch) const;
    QList<Packet> packetsByDirection(bool incoming) const;
    QList<Packet> packetsByTimeRange(qint64 from, qint64 to) const;
    int packetCount() const;
    void clear();
    bool exportToPcap(const QString &filePath);
    bool exportToCsv(const QString &filePath);
signals:
    void packetAdded(const Packet &pkt);
    void captureStarted();
    void captureStopped(int total);
    void exportFinished(const QString &path, bool ok);
private:
    QList<Packet> m_packets; bool m_capturing = false; int m_maxPackets = 10000;
};
