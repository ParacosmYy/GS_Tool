/**
 * @file DragDropHelper.h
 * @brief 拖放辅助工具 - 简化QWidget拖放操作的辅助类
 *
 * 提供声明式拖放配置，避免每个控件重复实现dragEnterEvent/dropEvent。
 * 支持文件拖放、文本拖放和自定义MIME类型。
 *
 * 用法:
 *   DragDropHelper::acceptFileDrop(widget, ".hex;.bin;.elf",
 *       [](const QStringList& files) { // 处理文件 });
 */

#ifndef DRAG_DROP_HELPER_H
#define DRAG_DROP_HELPER_H

#include <QObject>
#include <QStringList>
#include <QMimeType>
#include <functional>

class QWidget;
class QDragEnterEvent;
class QDropEvent;
class QMimeData;

/**
 * @brief 拖放类型
 */
enum class DropType {
    Files,          ///< 文件拖放
    Text,           ///< 纯文本拖放
    Urls,           ///< URL拖放
    Custom          ///< 自定义MIME
};

/**
 * @brief 拖放辅助工具(静态方法)
 *
 * 为任意QWidget安装拖放功能，通过事件过滤器实现。
 * 无需子类化QWidget。
 */
class DragDropHelper : public QObject {
    Q_OBJECT

public:
    /// 文件拖放回调
    using FileCallback = std::function<void(const QStringList& files)>;
    /// 文本拖放回调
    using TextCallback = std::function<void(const QString& text)>;
    /// URL拖放回调
    using UrlCallback = std::function<void(const QStringList& urls)>;

    /**
     * @brief 安装文件拖放
     * @param widget 目标控件
     * @param extensions 接受的文件扩展名(如 ".hex;.bin;.elf")，空=接受所有
     * @param callback 文件接收回调
     */
    static void acceptFileDrop(QWidget* widget,
                                const QString& extensions,
                                FileCallback callback);

    /**
     * @brief 安装文本拖放
     * @param widget 目标控件
     * @param callback 文本接收回调
     */
    static void acceptTextDrop(QWidget* widget, TextCallback callback);

    /**
     * @brief 安装URL拖放
     * @param widget 目标控件
     * @param callback URL接收回调
     */
    static void acceptUrlDrop(QWidget* widget, UrlCallback callback);

    /**
     * @brief 移除拖放功能
     * @param widget 目标控件
     */
    static void removeDropTarget(QWidget* widget);

    /**
     * @brief 设置拖放高亮样式(进入时添加边框高亮)
     * @param widget 目标控件
     * @param highlightColor 高亮颜色(如 "#4a9eff")
     * @param borderWidth 边框宽度(px)
     */
    static void setDropHighlight(QWidget* widget,
                                   const QString& highlightColor = "#4a9eff",
                                   int borderWidth = 2);

private:
    explicit DragDropHelper(QWidget* parent);
    ~DragDropHelper() override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    /// 处理拖入事件
    bool handleDragEnter(QDragEnterEvent* event);
    /// 处理放下事件
    bool handleDrop(QDropEvent* event);

    /// 检查文件扩展名是否匹配
    bool matchesExtension(const QString& filePath) const;

    QWidget* m_target{nullptr};
    DropType m_type{DropType::Files};
    QString m_extensions;
    QString m_highlightColor;
    int m_borderWidth{2};
    QString m_originalStyle;

    FileCallback m_fileCallback;
    TextCallback m_textCallback;
    UrlCallback m_urlCallback;

    // ---- 统计计数器(静态，跨所有实例累积) ----
    static inline quint64 s_totalDragEnters = 0;     ///< 累计拖入事件次数
    static inline quint64 s_totalDrops = 0;          ///< 累计放下事件次数
    static inline quint64 s_totalDropsRejected = 0;  ///< 累计拒绝放下事件次数
    static inline quint64 s_totalTargetsInstalled = 0; ///< 累计安装拖放目标次数
public:
    /** @brief 获取累计拖入事件次数 */
    static quint64 totalDragEnters() { return s_totalDragEnters; }
    /** @brief 获取累计放下事件次数 */
    static quint64 totalDrops() { return s_totalDrops; }
    /** @brief 获取累计拒绝放下事件次数 */
    static quint64 totalDropsRejected() { return s_totalDropsRejected; }
    /** @brief 获取累计安装拖放目标次数 */
    static quint64 totalTargetsInstalled() { return s_totalTargetsInstalled; }
    /** @brief 重置拖放统计计数器 */
    static void resetDragDropStatistics() { s_totalDragEnters = 0; s_totalDrops = 0; s_totalDropsRejected = 0; s_totalTargetsInstalled = 0; }
};

#endif // DRAG_DROP_HELPER_H
