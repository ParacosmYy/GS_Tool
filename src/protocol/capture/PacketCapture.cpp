#include "protocol/capture/PacketCapture.h"
#include <QTextStream>
#include <QDateTime>

PacketCapture::PacketCapture(QObject *parent) : QObject(parent) {}
PacketCapture::~PacketCapture() { stopCapture(); }

void PacketCapture::startCapture(int maxPackets) {
    m_maxPackets = maxPackets;
    m_capturing = true;
    emit captureStarted();
}

void PacketCapture::stopCapture() {
    if (m_capturing) {
        m_capturing = false;
        emit captureStopped(m_packets.size());
    }
}

bool PacketCapture::isCapturing() const { return m_capturing; }

void PacketCapture::addPacket(const QByteArray &data, bool incoming, int channel) {
    if (!m_capturing) return;
    Packet pkt;
    pkt.data = data;
    pkt.timestamp = QDateTime::currentMSecsSinceEpoch();
    pkt.isIncoming = incoming;
    pkt.channel = channel;
    m_packets.append(pkt);
    if (m_packets.size() > m_maxPackets) m_packets.removeFirst();
    emit packetAdded(pkt);
}

QList<PacketCapture::Packet> PacketCapture::packets() const { return m_packets; }

QList<PacketCapture::Packet> PacketCapture::packetsByChannel(int ch) const {
    QList<Packet> result;
    for (const auto &p : m_packets) if (p.channel == ch) result.append(p);
    return result;
}

QList<PacketCapture::Packet> PacketCapture::packetsByDirection(bool incoming) const {
    QList<Packet> result;
    for (const auto &p : m_packets) if (p.isIncoming == incoming) result.append(p);
    return result;
}

QList<PacketCapture::Packet> PacketCapture::packetsByTimeRange(qint64 from, qint64 to) const {
    QList<Packet> result;
    for (const auto &p : m_packets) if (p.timestamp >= from && p.timestamp <= to) result.append(p);
    return result;
}

int PacketCapture::packetCount() const { return m_packets.size(); }
void PacketCapture::clear() { m_packets.clear(); }

bool PacketCapture::exportToPcap(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) { emit exportFinished(filePath, false); return false; }
    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("\xd4\xc3\xb2\xa1", 4);
    out << quint16(2) << quint16(4);
    out << qint32(0) << quint32(0);
    out << quint32(0x7FFFFFFF);
    out << quint32(1);
    for (const auto &pkt : m_packets) {
        qint64 sec = pkt.timestamp / 1000;
        qint64 usec = (pkt.timestamp % 1000) * 1000;
        out << quint32(sec) << quint32(usec);
        out << quint32(pkt.data.size()) << quint32(pkt.data.size());
        out.writeRawData(pkt.data.constData(), pkt.data.size());
    }
    file.close();
    emit exportFinished(filePath, true);
    return true;
}

bool PacketCapture::exportToCsv(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { emit exportFinished(filePath, false); return false; }
    QTextStream out(&file);
    out << "timestamp,direction,channel,size,hex_data\n";
    for (const auto &pkt : m_packets) {
        out << pkt.timestamp << ","
            << (pkt.isIncoming ? "IN" : "OUT") << ","
            << pkt.channel << ","
            << pkt.data.size() << ","
            << pkt.data.toHex() << "\n";
    }
    file.close();
    emit exportFinished(filePath, true);
    return true;
}
