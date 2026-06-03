#include "protocol/capture/PacketCapture.h"
#include <QTextStream>
#include <QDateTime>
#include <QDataStream>
PacketCapture::PacketCapture(QObject *parent) : QObject(parent) {}
PacketCapture::~PacketCapture() { stopCapture(); }
void PacketCapture::startCapture(int max) { m_maxPackets = max; m_capturing = true; emit captureStarted(); }
void PacketCapture::stopCapture() { if (m_capturing) { m_capturing = false; emit captureStopped(m_packets.size()); } }
bool PacketCapture::isCapturing() const { return m_capturing; }
void PacketCapture::addPacket(const QByteArray &data, bool incoming, int channel) {
    if (!m_capturing) return;
    Packet pkt; pkt.data = data; pkt.timestamp = QDateTime::currentMSecsSinceEpoch(); pkt.isIncoming = incoming; pkt.channel = channel;
    m_packets.append(pkt);
    if (m_packets.size() > m_maxPackets) m_packets.removeFirst();
    emit packetAdded(pkt);
}
QList<PacketCapture::Packet> PacketCapture::packets() const { return m_packets; }
QList<PacketCapture::Packet> PacketCapture::packetsByChannel(int ch) const { QList<Packet> r; for (const auto &p : m_packets) if (p.channel == ch) r.append(p); return r; }
QList<PacketCapture::Packet> PacketCapture::packetsByDirection(bool inc) const { QList<Packet> r; for (const auto &p : m_packets) if (p.isIncoming == inc) r.append(p); return r; }
QList<PacketCapture::Packet> PacketCapture::packetsByTimeRange(qint64 from, qint64 to) const { QList<Packet> r; for (const auto &p : m_packets) if (p.timestamp >= from && p.timestamp <= to) r.append(p); return r; }
int PacketCapture::packetCount() const { return m_packets.size(); }
void PacketCapture::clear() { m_packets.clear(); }
bool PacketCapture::exportToPcap(const QString &fp) {
    QFile f(fp); if (!f.open(QIODevice::WriteOnly)) { emit exportFinished(fp, false); return false; }
    QDataStream out(&f); out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("\xd4\xc3\xb2\xa1", 4); out << quint16(2) << quint16(4) << qint32(0) << quint32(0) << quint32(0x7FFFFFFF) << quint32(1);
    for (const auto &pkt : m_packets) { out << quint32(pkt.timestamp/1000) << quint32((pkt.timestamp%1000)*1000) << quint32(pkt.data.size()) << quint32(pkt.data.size()); out.writeRawData(pkt.data.constData(), pkt.data.size()); }
    f.close(); emit exportFinished(fp, true); return true;
}
bool PacketCapture::exportToCsv(const QString &fp) {
    QFile f(fp); if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { emit exportFinished(fp, false); return false; }
    QTextStream out(&f); out << "timestamp,direction,channel,size,hex_data\n";
    for (const auto &pkt : m_packets) { out << pkt.timestamp << "," << (pkt.isIncoming?"IN":"OUT") << "," << pkt.channel << "," << pkt.data.size() << "," << pkt.data.toHex() << "\n"; }
    f.close(); emit exportFinished(fp, true); return true;
}
