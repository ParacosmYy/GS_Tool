/**
 * @file DragDropHelper2.cpp
 * @brief 拖放辅助器v2实现 — MIME类型注册、拖放事件处理、内部拖拽数据管理
 */
#include "core/widgets2/DragDropHelper2.h"
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

/** @brief 构造函数 @param parent 父对象 */
DragDropHelper::DragDropHelper(QObject *parent) : QObject(parent) {}
/** @brief 析构函数 */
DragDropHelper::~DragDropHelper() = default;

/** @brief 注册MIME类型处理回调 @param mime MIME类型字符串 @param h 处理回调函数 */
void DragDropHelper::registerHandler(const QString &mime, DropHandler h) { m_handlers[mime] = h; ++m_totalHandlerRegistrations; }
/** @brief 注销MIME类型处理回调 @param mime MIME类型字符串 */
void DragDropHelper::unregisterHandler(const QString &mime) { m_handlers.remove(mime); }

/** @brief 处理拖放释放事件 — 匹配MIME处理器或纯文本回退 @param event 拖放事件 @return 成功处理返回true */
bool DragDropHelper::handleDrop(QDropEvent *event) {
    const auto *mime = event->mimeData();
    if (!mime) { ++m_totalRejectedDrops; emit dropRejected(tr("No data")); return false; }
    for (auto it = m_handlers.constBegin(); it != m_handlers.constEnd(); ++it) {
        if (mime->hasFormat(it.key())) {
            QByteArray data = mime->data(it.key());
            it.value()(data, it.key());
            ++m_totalSuccessfulDrops;
            emit dataDropped(it.key(), data);
            event->acceptProposedAction();
            return true;
        }
    }
    if (mime->hasText()) {
        QByteArray data = mime->text().toUtf8();
        ++m_totalSuccessfulDrops;
        emit dataDropped("text/plain", data);
        event->acceptProposedAction();
        return true;
    }
    ++m_totalRejectedDrops;
    emit dropRejected(tr("Unsupported format"));
    return false;
}

/** @brief 处理拖入事件 — 检查MIME类型是否在可接受列表中 @param event 拖入事件 @return 接受返回true */
bool DragDropHelper::handleDragEnter(QDragEnterEvent *event) {
    const auto *mime = event->mimeData();
    QStringList found;
    for (const auto &type : m_acceptedTypes) if (mime->hasFormat(type)) found << type;
    if (!found.isEmpty()) { ++m_totalDragEnters; event->acceptProposedAction(); emit dragEntered(found); return true; }
    if (mime->hasText()) { ++m_totalDragEnters; event->acceptProposedAction(); return true; }
    return false;
}

/** @brief 设置可接受的MIME类型列表 @param t MIME类型列表 */
void DragDropHelper::setAcceptedMimeTypes(const QStringList &t) { m_acceptedTypes = t; }
/** @brief 获取可接受的MIME类型列表 @return MIME类型列表 */
QStringList DragDropHelper::acceptedMimeTypes() const { return m_acceptedTypes; }
/** @brief 启用/禁用内部拖拽模式 @param e true启用 */
void DragDropHelper::enableInternalDrag(bool e) { m_internalDrag = e; }
/** @brief 设置内部拖拽数据 @param mime MIME类型 @param d 数据内容 */
void DragDropHelper::setDragData(const QString &mime, const QByteArray &d) { m_dragData[mime] = d; }
/** @brief 获取内部拖拽数据 @param mime MIME类型 @return 数据内容 */
QByteArray DragDropHelper::dragData(const QString &mime) const { return m_dragData.value(mime); }
