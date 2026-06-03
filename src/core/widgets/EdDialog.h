/**
 * @file EdDialog.h
 * @brief 自定义对话框组件 — 统一替代 QMessageBox 的语义化弹窗
 *
 * 支持三种语义类型: Confirm(确认)/Warning(警告)/Error(错误)
 * 动画: 200ms OutCubic 淡入+缩放(95%→100%), 关闭时反向动画
 * 颜色全部从 ThemeManager 获取, 通过 paintEvent 自绘, 无 QSS 依赖
 *
 * 使用:
 *   bool ok = EdDialog::confirm(this, tr("删除"), tr("确定删除吗？"));
 *   EdDialog::warning(this, tr("警告"), tr("输入不能为空"));
 *   EdDialog::error(this, tr("错误"), tr("无法写入文件"));
 */

#ifndef EDDIALOG_H
#define EDDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

/**
 * @brief 自定义对话框 — 统一确认/警告/错误三种弹窗样式
 *
 * 通过 paintEvent 自绘背景和左侧边框强调色，按钮通过 objectName 供 QSS 定位。
 * 静态便捷方法自动创建实例、exec()、返回结果。
 */
class EdDialog : public QDialog {
    Q_OBJECT

public:
    /** @brief 对话框语义类型 */
    enum class DialogType {
        Confirm,   ///< 确认对话框 — 双按钮(取消/确认), accent 色确认按钮
        Warning,   ///< 警告对话框 — 双按钮(取消/确认), 左侧 Warning 色边框
        Error      ///< 错误对话框 — 单按钮(确认), 左侧 Error 色边框
    };

    /**
     * @brief 构造对话框
     * @param parent 父窗口
     * @param title 标题文本
     * @param description 描述文本
     * @param type 对话框类型
     */
    explicit EdDialog(QWidget* parent, const QString& title,
                      const QString& description, DialogType type);

    /**
     * @brief 显示确认对话框并返回用户选择
     * @param parent 父窗口
     * @param title 标题
     * @param description 描述
     * @return true 用户点击了确认按钮
     */
    static bool confirm(QWidget* parent, const QString& title,
                        const QString& description);

    /**
     * @brief 显示警告对话框
     * @param parent 父窗口
     * @param title 标题
     * @param description 描述
     */
    static void warning(QWidget* parent, const QString& title,
                        const QString& description);

    /**
     * @brief 显示错误对话框
     * @param parent 父窗口
     * @param title 标题
     * @param description 描述
     */
    static void error(QWidget* parent, const QString& title,
                      const QString& description);

    // ── 统计计数器 ──

    /** @brief 获取对话框总打开次数 */
    static quint64 totalDialogOpens() { return s_totalDialogOpens; }
    /** @brief 获取对话框总关闭次数 */
    static quint64 totalDialogCloses() { return s_totalDialogCloses; }
    /** @brief 重置所有统计计数器 */
    static void resetDialogStatistics() { s_totalDialogOpens = 0; s_totalDialogCloses = 0; }

protected:
    /** @brief 自绘: 背景圆角 + 左侧强调色边框 + 图标 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 显示时触发 200ms OutCubic 淡入+缩放动画 */
    void showEvent(QShowEvent* event) override;

    /** @brief Escape 键关闭对话框 */
    void keyPressEvent(QKeyEvent* event) override;

private:
    /** @brief 获取当前类型的语义强调色 */
    QColor accentColor() const;

    /** @brief 获取当前类型的图标 Unicode 字符 */
    QString iconChar() const;

    /** @brief 执行关闭动画后删除对话框 */
    void closeWithAnimation();

    DialogType m_type;                                  ///< 对话框类型
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;  ///< 淡入淡出特效
    QPushButton* m_confirmBtn = nullptr;                ///< 确认按钮
    int m_resultCode = QDialog::Rejected;               ///< 对话框结果

    static constexpr int kRadius = 8;         ///< 圆角半径
    static constexpr int kLeftBorder = 3;     ///< 左侧强调色边框宽度
    static constexpr int kPadding = 24;       ///< 内边距
    static constexpr int kButtonHeight = 32;  ///< 按钮高度
    static constexpr int kButtonMinWidth = 80;///< 按钮最小宽度
    static constexpr int kButtonSpacing = 8;  ///< 按钮间距
    static constexpr int kIconSize = 18;      ///< 图标字号
    static constexpr int kIconArea = 28;      ///< 图标区域宽度

    // ── 统计计数器(static inline，因为静态工厂模式创建临时实例) ──
    static inline quint64 s_totalDialogOpens = 0;   ///< 对话框总打开次数
    static inline quint64 s_totalDialogCloses = 0;  ///< 对话框总关闭次数
};

#endif // EDDIALOG_H
