/**
 * @file EmptyStateWidget.h
 * @brief 空状态组件 — 当面板无数据/无连接/搜索无结果时显示的占位界面
 *
 * 居中垂直排列: [icon(48px)] → [title(bold 14px)] → [description(muted 12px)] → [action button(optional)]
 * 最大宽度 320px，使用 ThemeManager 语义色。
 */
#ifndef EMPTY_STATE_WIDGET_H
#define EMPTY_STATE_WIDGET_H

#include <QWidget>
#include <functional>

class QLabel;
class QPushButton;
class QVBoxLayout;

/**
 * @brief 空状态显示组件
 *
 * 使用场景:
 *   - 无连接时显示"暂无连接"
 *   - 无数据时显示"暂无数据"
 *   - 搜索无结果时显示"未找到匹配项"
 *   - 面板加载中占位
 */
class EmptyStateWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造空状态组件
     * @param title 标题文字(必须使用tr())
     * @param description 描述文字(可选, 使用tr())
     * @param parent 父控件
     */
    explicit EmptyStateWidget(const QString& title,
                               const QString& description = QString(),
                               QWidget* parent = nullptr);

    /** @brief 设置图标名称(Lucide图标名，暂用文字占位) */
    void setIconName(const QString& name);
    /** @brief 设置标题 */
    void setTitle(const QString& title);
    /** @brief 设置描述文字 */
    void setDescription(const QString& description);
    /** @brief 设置操作按钮文字和回调，空字符串则隐藏按钮 */
    void setActionButton(const QString& text, std::function<void()> callback = nullptr);

    // ── 统计计数器 ──

    /** @brief 获取状态内容变更总次数(标题/描述) */
    quint64 totalStateChanges() const { return m_totalStateChanges; }
    /** @brief 获取图标变更总次数 */
    quint64 totalIconChanges() const { return m_totalIconChanges; }
    /** @brief 获取操作按钮点击总次数 */
    quint64 totalActionButtonClicks() const { return m_totalActionButtonClicks; }
    /** @brief 重置所有统计计数器 */
    void resetEmptyStateStatistics();

private:
    void setupUI(const QString& title, const QString& description);

    QLabel* m_iconLabel = nullptr;       ///< objectName="emptyStateIcon"
    QLabel* m_titleLabel = nullptr;      ///< objectName="emptyStateTitle"
    QLabel* m_descLabel = nullptr;       ///< objectName="emptyStateDescription"
    QPushButton* m_actionBtn = nullptr;  ///< objectName="emptyStateAction"
    QVBoxLayout* m_mainLayout = nullptr; ///< 主布局

    // ── 统计计数器 ──
    quint64 m_totalStateChanges = 0;     ///< 状态内容变更次数(标题/描述)
    quint64 m_totalIconChanges = 0;      ///< 图标变更次数
    quint64 m_totalActionButtonClicks = 0; ///< 操作按钮点击次数
};

#endif // EMPTY_STATE_WIDGET_H
