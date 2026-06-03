/**
 * @file DragDropHelper.cpp
 * @brief 拖放辅助工具实现
 */

#include "core/widgets/DragDropHelper.h"
#include "core/theme/ThemeManager.h"

#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDebug>

// ============================================================
// DragDropHelper 私有构造
// ============================================================

DragDropHelper::DragDropHelper(QWidget* parent)
    : QObject(parent)
    , m_target(parent)
{
    m_target->setAcceptDrops(true);
    m_target->installEventFilter(this);
}

DragDropHelper::~DragDropHelper()
{
    if (m_target) {
        m_target->removeEventFilter(this);
    }
}

// ============================================================
// 静态工厂方法
// ============================================================

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

    // 设置控件属性用于标识
    widget->setProperty("_dropType", "files");
    widget->setProperty("_dropExtensions", extensions);
}

void DragDropHelper::acceptTextDrop(QWidget* widget, TextCallback callback)
{
    if (!widget || !callback) return;

    auto* helper = new DragDropHelper(widget);
    helper->m_type = DropType::Text;
    helper->m_textCallback = std::move(callback);
    ++s_totalTargetsInstalled;

    widget->setProperty("_dropType", "text");
}

void DragDropHelper::acceptUrlDrop(QWidget* widget, UrlCallback callback)
{
    if (!widget || !callback) return;

    auto* helper = new DragDropHelper(widget);
    helper->m_type = DropType::Urls;
    helper->m_urlCallback = std::move(callback);
    ++s_totalTargetsInstalled;

    widget->setProperty("_dropType", "urls");
}

void DragDropHelper::removeDropTarget(QWidget* widget)
{
    if (!widget) return;
    // 查找并删除helper
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

void DragDropHelper::setDropHighlight(QWidget* widget,
                                        const QString& highlightColor,
                                        int borderWidth)
{
    if (!widget) return;
    widget->setProperty("_dropHighlightColor", highlightColor);
    widget->setProperty("_dropHighlightWidth", borderWidth);
}

// ============================================================
// 事件过滤器
// ============================================================

bool DragDropHelper::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_target) return false;

    switch (event->type()) {
    case QEvent::DragEnter:
        return handleDragEnter(static_cast<QDragEnterEvent*>(event));
    case QEvent::Drop:
        return handleDrop(static_cast<QDropEvent*>(event));
    case QEvent::DragLeave: {
        // 恢复原始样式
        if (!m_originalStyle.isEmpty()) {
            m_target->setStyleSheet(m_originalStyle);
        }
        return true;
    }
    default:
        return false;
    }
}

bool DragDropHelper::handleDragEnter(QDragEnterEvent* event)
{
    const auto* mime = event->mimeData();
    if (!mime) return false;

    bool accepted = false;

    switch (m_type) {
    case DropType::Files: {
        if (mime->hasUrls()) {
            auto urls = mime->urls();
            for (const auto& url : urls) {
                if (url.isLocalFile()) {
                    if (matchesExtension(url.toLocalFile())) {
                        accepted = true;
                        break;
                    }
                }
            }
        }
        break;
    }
    case DropType::Text:
        accepted = mime->hasText();
        break;
    case DropType::Urls:
        accepted = mime->hasUrls();
        break;
    case DropType::Custom:
        break;
    }

    if (accepted) {
        event->acceptProposedAction();
        ++s_totalDragEnters;
        // 应用高亮 — 默认使用主题强调色
        QString color = m_target->property("_dropHighlightColor").toString();
        if (color.isEmpty()) color = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent).name();
        int bw = m_target->property("_dropHighlightWidth").toInt();
        if (bw <= 0) bw = 2;

        m_originalStyle = m_target->styleSheet();
        m_target->setStyleSheet(
            m_originalStyle +
            QString("\n#%1 { border: %2px solid %3; border-radius: 4px; }")
                .arg(m_target->objectName().isEmpty() ? "*" : m_target->objectName())
                .arg(bw)
                .arg(color));
    }

    return accepted;
}

bool DragDropHelper::handleDrop(QDropEvent* event)
{
    const auto* mime = event->mimeData();
    if (!mime) return false;

    // 恢复样式
    if (!m_originalStyle.isEmpty()) {
        m_target->setStyleSheet(m_originalStyle);
    }

    switch (m_type) {
    case DropType::Files: {
        QStringList files;
        auto urls = mime->urls();
        for (const auto& url : urls) {
            if (url.isLocalFile() && matchesExtension(url.toLocalFile())) {
                files.append(url.toLocalFile());
            }
        }
        if (!files.isEmpty() && m_fileCallback) {
            m_fileCallback(files);
            ++s_totalDrops;
            event->acceptProposedAction();
            return true;
        }
        break;
    }
    case DropType::Text: {
        QString text = mime->text();
        if (!text.isEmpty() && m_textCallback) {
            m_textCallback(text);
            ++s_totalDrops;
            event->acceptProposedAction();
            return true;
        }
        break;
    }
    case DropType::Urls: {
        QStringList urls;
        for (const auto& url : mime->urls()) {
            urls.append(url.toString());
        }
        if (!urls.isEmpty() && m_urlCallback) {
            m_urlCallback(urls);
            ++s_totalDrops;
            event->acceptProposedAction();
            return true;
        }
        break;
    }
    case DropType::Custom:
        break;
    }

    return false;
}

bool DragDropHelper::matchesExtension(const QString& filePath) const
{
    if (m_extensions.isEmpty()) return true;

    QString fileExt;
    int dotPos = filePath.lastIndexOf('.');
    if (dotPos >= 0) {
        fileExt = filePath.mid(dotPos).toLower();
    }

    auto exts = m_extensions.split(';', Qt::SkipEmptyParts);
    for (const auto& ext : exts) {
        if (fileExt == ext.trimmed().toLower()) {
            return true;
        }
    }
    return false;
}
