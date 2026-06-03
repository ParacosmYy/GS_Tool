/**
 * @file DragDropHelper2.h
 * @brief 拖放辅助工具，统一管理MIME类型拖放事件的分发和处理
 */
#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <functional>

class QMimeData;
class QDropEvent;
class QDragEnterEvent;

/**
 * @class DragDropHelper
 * @brief 拖放辅助工具，按MIME类型注册处理器并自动分发拖放事件
 */
class DragDropHelper : public QObject {
    Q_OBJECT
public:
    /** @brief 拖放处理回调类型，接收数据及其MIME格式 */
    using DropHandler = std::function<void(const QByteArray &data, const QString &format)>;

    /** @brief 构造函数 @param parent 父对象指针 */
    explicit DragDropHelper(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~DragDropHelper() override;

    /** @brief 注册指定MIME类型的拖放处理器 @param mimeType MIME类型 @param handler 处理回调 */
    void registerHandler(const QString &mimeType, DropHandler handler);
    /** @brief 注销指定MIME类型的处理器 @param mimeType MIME类型 */
    void unregisterHandler(const QString &mimeType);
    /** @brief 处理拖放释放事件 @param event 拖放事件 @return 是否成功处理 */
    bool handleDrop(QDropEvent *event);
    /** @brief 处理拖入进入事件 @param event 拖入事件 @return 是否接受 */
    bool handleDragEnter(QDragEnterEvent *event);
    /** @brief 设置接受的MIME类型列表 @param types MIME类型列表 */
    void setAcceptedMimeTypes(const QStringList &types);
    /** @brief 获取当前接受的MIME类型列表 @return MIME类型列表 */
    QStringList acceptedMimeTypes() const;
    /** @brief 启用或禁用内部拖拽模式 @param enable 是否启用 */
    void enableInternalDrag(bool enable);
    /** @brief 设置内部拖拽数据 @param mimeType MIME类型 @param data 拖拽数据 */
    void setDragData(const QString &mimeType, const QByteArray &data);
    /** @brief 获取内部拖拽数据 @param mimeType MIME类型 @return 拖拽数据 */
    QByteArray dragData(const QString &mimeType) const;

signals:
    /** @brief 数据被成功放下时发射 @param mimeType MIME类型 @param data 数据内容 */
    void dataDropped(const QString &mimeType, const QByteArray &data);
    /** @brief 拖拽进入控件时发射 @param mimeTypes 拖入的MIME类型列表 */
    void dragEntered(const QStringList &mimeTypes);
    /** @brief 拖放被拒绝时发射 @param reason 拒绝原因 */
    void dropRejected(const QString &reason);

private:
    QMap<QString, DropHandler> m_handlers;     ///< MIME类型到处理回调的映射
    QStringList m_acceptedTypes;               ///< 接受的MIME类型列表
    QMap<QString, QByteArray> m_dragData;      ///< 内部拖拽数据
    bool m_internalDrag = false;               ///< 是否为内部拖拽模式
};
