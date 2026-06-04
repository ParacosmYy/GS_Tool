/**
 * @file DragDropHelper.cpp
 * @brief 拖放辅助工具实现 — 文件/文本/URL拖放支持+高亮反馈
 */

#include "core/widgets/DragDropHelper.h"
#include "core/theme/ThemeManager.h"

#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDebug>

// ─── 构造/析构 ───────────────────────────────────────────

/** @brief 构造函数 - 安装事件过滤器到目标控件 @param parent 目标控件 */
DragDropHelper::DragDropHelper(QWidget* parent)
    : QObject(parent)
    , m_target(parent)
{
    m_target->setAcceptDrops(true);
    m_target->installEventFilter(this);
}

/** @brief 析构函数 - 移除事件过滤器 */
DragDropHelper::~DragDropHelper()
{
    if (m_target) {
        m_target->removeEventFilter(this);
    }
}

// ─── 静态工厂方法 ────────────────────────────────────────

/** @brief 为控件启用文件拖放 @param widget 目标控件 @param extensions 允许的扩展名(分号分隔) @param callback 文件回调 */
void DragDropHelper::acceptFileDrop(QWidget* widget,
                                       const QString& extensions,
                                       FileCallback callback)
{
    if (!widget || !callback) return;

    auto* helper = new DragDropHelper(widget);
    helper->m_type = DropType::Files;
    helper->m_extensions = extensions.toLower();
    helper->m_fileCallback = std::move(callback);
    ++s_totalTargetsInstalled;

    widget->setProperty("_dropType", "files");
    widget->setProperty("_dropExtensions", extensions);
}

/** @brief 为控件启用文本拖放 @param widget 目标控件 @param callback 文本回调 */
void DragDropHelper::acceptTextDrop(QWidget* widget, TextCallback callback)
{
    if (!widget || !callback) return;

    auto* helper = new DragDropHelper(widget);
    helper->m_type = DropType::Text;
    helper->m_textCallback = std::move(callback);
    ++s_totalTargetsInstalled;

    widget->setProperty("_dropType", "text");
}

/** @brief 为控件启用URL拖放 @param widget 目标控件 @param callback URL回调 */
void DragDropHelper::acceptUrlDrop(QWidget* widget, UrlCallback callback)
{
    if (!widget || !callback) return;

    auto* helper = new DragDropHelper(widget);
    helper->m_type = DropType::Urls;
    helper->m_urlCallback = std::move(callback);
    ++s_totalTargetsInstalled;

    widget->setProperty("_dropType", "urls");
}

/** @brief 移除控件的拖放支持 @param widget 目标控件 */
void DragDropHelper::removeDropTarget(QWidget* widget)
{
    if (!widget) return;
    auto children = widget->children();
    for (auto* child : children) {
        auto* helper = qobject_cast<DragDropHelper*>(child);
        if (helper) {
            delete helper;
            break;
        }
    }
    widget->setAcceptDrops(false);
}

/** @brief 设置拖放高亮样式 @param widget 目标控件 @param highlightColor 高亮颜色 @param borderWidth 边框宽度 */
void DragDropHelper::setDropHighlight(QWidget* widget,
                                        const QString& highlightColor,
                                        int borderWidth)
{
    if (!widget) return;
    widget->setProperty("_dropHighlightColor", highlightColor);
    widget->setProperty("_dropHighlightWidth", borderWidth);
}

// 事件过滤器/拖放处理见 DragDropHelperEvents.cpp
