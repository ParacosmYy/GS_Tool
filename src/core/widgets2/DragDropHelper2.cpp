#include "core/widgets2/DragDropHelper2.h"
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

DragDropHelper::DragDropHelper(QObject *parent) : QObject(parent) {}
DragDropHelper::~DragDropHelper() = default;

void DragDropHelper::registerHandler(const QString &mime, DropHandler h) { m_handlers[mime] = h; }
void DragDropHelper::unregisterHandler(const QString &mime) { m_handlers.remove(mime); }

bool DragDropHelper::handleDrop(QDropEvent *event) {
    const auto *mime = event->mimeData();
    if (!mime) { emit dropRejected(tr("No data")); return false; }
    for (auto it = m_handlers.constBegin(); it != m_handlers.constEnd(); ++it) {
        if (mime->hasFormat(it.key())) {
            QByteArray data = mime->data(it.key());
            it.value()(data, it.key());
            emit dataDropped(it.key(), data);
            event->acceptProposedAction();
            return true;
        }
    }
    if (mime->hasText()) {
        QByteArray data = mime->text().toUtf8();
        emit dataDropped("text/plain", data);
        event->acceptProposedAction();
        return true;
    }
    emit dropRejected(tr("Unsupported format"));
    return false;
}

bool DragDropHelper::handleDragEnter(QDragEnterEvent *event) {
    const auto *mime = event->mimeData();
    QStringList found;
    for (const auto &type : m_acceptedTypes) if (mime->hasFormat(type)) found << type;
    if (!found.isEmpty()) { event->acceptProposedAction(); emit dragEntered(found); return true; }
    if (mime->hasText()) { event->acceptProposedAction(); return true; }
    return false;
}

void DragDropHelper::setAcceptedMimeTypes(const QStringList &t) { m_acceptedTypes = t; }
QStringList DragDropHelper::acceptedMimeTypes() const { return m_acceptedTypes; }
void DragDropHelper::enableInternalDrag(bool e) { m_internalDrag = e; }
void DragDropHelper::setDragData(const QString &mime, const QByteArray &d) { m_dragData[mime] = d; }
QByteArray DragDropHelper::dragData(const QString &mime) const { return m_dragData.value(mime); }
