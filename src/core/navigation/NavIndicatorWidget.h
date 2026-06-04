/** @file NavIndicatorWidget.h @brief 导航树选中滑动指示器 -- 左侧accent色竖线的平滑滑动动画(250ms OutCubic)。QPropertyAnimation驱动indicatorY, ThemeManager取色 */

#ifndef NAV_INDICATOR_WIDGET_H
#define NAV_INDICATOR_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>
#include <QEasingCurve>
#include <QTreeView>

#include "core/theme/ThemeManager.h"
#include "shared/AnimationConstants.h"

/** @brief 导航树选中滑动指示器。透明覆盖层，在导航树左侧绘制3px宽accent色竖线，选中项切换时平滑滑动 */
class NavIndicatorWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal indicatorY READ indicatorY WRITE setIndicatorY NOTIFY indicatorYChanged)

public:
    explicit NavIndicatorWidget(QTreeView* navTree, QWidget* parent = nullptr) ///< 构造(作为navTree子控件覆盖层)
        : QWidget(navTree)
        , m_navTree(navTree)
        , m_indicatorY(0.0)
        , m_indicatorHeight(0)
    {
        // 基本属性
        setObjectName("navIndicator");

        // 透明背景，不拦截鼠标事件（让点击穿透到 navTree）
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);  // 不自动填充背景，保持透明

        // 初始化滑动动画（250ms OutCubic，符合 CLAUDE.md §6.5 规范）
        m_slideAnim = new QPropertyAnimation(this, "indicatorY", this);
        m_slideAnim->setDuration(Animations::kNavIndicatorMs);
        m_slideAnim->setEasingCurve(QEasingCurve::OutCubic);

        // 安装事件过滤器，跟随 navTree 尺寸变化自动调整大小
        m_navTree->installEventFilter(this);

        // 监听主题切换，刷新指示线颜色
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
                this, &NavIndicatorWidget::updateThemeColor);
    }

    void animateTo(const QModelIndex& index) { moveToIndex(index); } ///< 动画滑动(moveToIndex别名)
    /** 动画滑动到指定索引(250ms OutCubic)，动画运行中从当前位置衔接 @param index 目标模型索引 */
    void moveToIndex(const QModelIndex& index)
    {
        QRect visualRect = m_navTree->visualRect(index);
        qreal targetY = visualRect.y();
        int targetHeight = visualRect.height();

        // 防止无效矩形（如项不可见或模型为空）
        if (targetHeight <= 0) return;

        m_indicatorHeight = targetHeight;

        // 如果动画正在运行，从当前位置开始新动画（避免跳变）
        qreal startY = m_indicatorY;
        if (m_slideAnim->state() == QAbstractAnimation::Running) {
            startY = m_slideAnim->currentValue().toReal();
        }

        m_slideAnim->stop();
        m_slideAnim->setStartValue(startY);
        m_slideAnim->setEndValue(targetY);
        m_slideAnim->start();
        ++m_totalAnimations;
        ++m_totalPositionChanges;
    }

    /** 无动画跳转到指定索引(用于初始化和恢复会话) @param index 目标模型索引 */
    void jumpToIndex(const QModelIndex& index)
    {
        QRect visualRect = m_navTree->visualRect(index);
        if (visualRect.height() <= 0) return;

        m_indicatorHeight = visualRect.height();
        m_indicatorY = visualRect.y();
        ++m_totalPositionChanges;
        ++m_totalJumpMoves;
        update();
    }

    qreal indicatorY() const { return m_indicatorY; } ///< 获取指示线Y坐标

    // ── 统计计数器 ──
    quint64 totalAnimations() const { return m_totalAnimations; } ///< 动画启动总次数
    quint64 totalPositionChanges() const { return m_totalPositionChanges; } ///< 位置变更总次数
    quint64 totalJumpMoves() const { return m_totalJumpMoves; } ///< 无动画跳转总次数(初始化/恢复会话)
    quint64 totalThemeUpdates() const { return m_totalThemeUpdates; } ///< 主题颜色更新总次数
    void resetIndicatorStatistics(); ///< 重置统计

    /** 设置指示线Y坐标(QPropertyAnimation写访问器) @param y 目标Y坐标 */
    void setIndicatorY(qreal y)
    {
        if (qFuzzyCompare(m_indicatorY, y)) return;
        m_indicatorY = y;
        emit indicatorYChanged(y);
        update();
    }

signals:
    void indicatorYChanged(qreal y);         ///< 指示线Y坐标变化信号

public slots:
    void updateThemeColor() { ++m_totalThemeUpdates; update(); }    ///< 主题切换时刷新颜色

protected:
    /** 绘制指示线(左侧3px宽accent色竖线，上下圆角) @param event 绘制事件 */
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        // 防止高度无效时绘制
        if (m_indicatorHeight <= 0) return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // 从 ThemeManager 获取 Accent 颜色（无硬编码色值）
        QColor accentColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);

        // 指示线参数: 宽 3px，左侧对齐，上下圆角
        constexpr int kLineWidth = 3;
        constexpr int kRadius = 2;

        // 计算绘制区域（考虑 header 偏移）
        int headerHeight = m_navTree->header()->height();
        qreal drawY = m_indicatorY + headerHeight;

        QRectF indicatorRect(0, drawY, kLineWidth, m_indicatorHeight);

        painter.setPen(Qt::NoPen);
        painter.setBrush(accentColor);
        painter.drawRoundedRect(indicatorRect, kRadius, kRadius);
    }

    /** 事件过滤器(监听navTree的resize同步调整自身大小) @param watched 被监听对象 @param event 事件对象 */
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == m_navTree && event->type() == QEvent::Resize) {
            // 跟随 navTree 尺寸变化，覆盖整个树区域
            setGeometry(0, 0, m_navTree->width(), m_navTree->height());
        }
        return QWidget::eventFilter(watched, event);
    }

private:
    QTreeView* m_navTree;              ///< 关联的导航树控件
    QPropertyAnimation* m_slideAnim;   ///< 滑动动画（250ms OutCubic）
    qreal m_indicatorY;                ///< 指示线顶部 Y 坐标
    int m_indicatorHeight;             ///< 指示线高度（跟随选中项行高）

    // ── 统计计数器 ──
    quint64 m_totalAnimations = 0;     ///< 动画启动次数
    quint64 m_totalPositionChanges = 0;///< 位置变更次数
    quint64 m_totalJumpMoves = 0;      ///< 无动画跳转次数
    quint64 m_totalThemeUpdates = 0;   ///< 主题颜色更新次数
};

/** @brief 重置导航指示器统计计数器(内联实现) */
inline void NavIndicatorWidget::resetIndicatorStatistics()
{
    m_totalAnimations = 0;
    m_totalPositionChanges = 0;
    m_totalJumpMoves = 0;
    m_totalThemeUpdates = 0;
}

#endif // NAV_INDICATOR_WIDGET_H
