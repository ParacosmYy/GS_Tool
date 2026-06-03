#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <functional>

class QMimeData;
class QDropEvent;
class QDragEnterEvent;

class DragDropHelper : public QObject {
    Q_OBJECT
public:
    using DropHandler = std::function<void(const QByteArray &data, const QString &format)>;
    explicit DragDropHelper(QObject *parent = nullptr);
    ~DragDropHelper() override;
    void registerHandler(const QString &mimeType, DropHandler handler);
    void unregisterHandler(const QString &mimeType);
    bool handleDrop(QDropEvent *event);
    bool handleDragEnter(QDragEnterEvent *event);
    void setAcceptedMimeTypes(const QStringList &types);
    QStringList acceptedMimeTypes() const;
    void enableInternalDrag(bool enable);
    void setDragData(const QString &mimeType, const QByteArray &data);
    QByteArray dragData(const QString &mimeType) const;
signals:
    void dataDropped(const QString &mimeType, const QByteArray &data);
    void dragEntered(const QStringList &mimeTypes);
    void dropRejected(const QString &reason);
private:
    QMap<QString, DropHandler> m_handlers;
    QStringList m_acceptedTypes;
    QMap<QString, QByteArray> m_dragData;
    bool m_internalDrag = false;
};
