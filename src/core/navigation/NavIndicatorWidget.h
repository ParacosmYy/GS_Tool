/**
 * @file NavIndicatorWidget.h
 * @brief 导航树选中滑动指示器 -- 左侧 accent 色竖线的平滑滑动动画(250ms OutCubic)
 *
 * 设计: navTree子控件覆盖层, QPropertyAnimation驱动indicatorY, ThemeManager取色
 * 协作: QTreeView(父控件) / ThemeManager(Accent色) / MainWindow(信号连接)
 */

#ifndef NAV_INDICATOR_WIDGET_H
#define NAV_INDICATOR_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>
#include <QEasingCurve>
#include <QTreeView>

#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

/**
 * @brief 导航树选中滑动指示器
 *
 * 透明覆盖层，在导航树左侧绘制 3px 宽的 accent 色竖线。
 * 选中项切换时，竖线通过 QPropertyAnimation 平滑滑动。
 *
 * 属性:
 *   indicatorY - 指示线顶部 Y 坐标（QPropertyAnimation 驱动）
 */
class NavIndicatorWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal indicatorY READ indicatorY WRITE setIndicatorY NOTIFY indicatorYChanged)

public:
    /** @brief 构造指示器，作为navTree的子控件覆盖在其上方，初始化滑动动画和事件过滤 @param navTree 关联的导航树控件(指示器成为其子控件) @param parent 父控件(通常为nullptr，因为实际父级设为navTree) */
    explicit NavIndicatorWidget(QTreeView* navTree, QWidget* parent = nullptr)
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

    /** @brief 动画滑动到指定索引(moveToIndex的语义别名，便于信号/槽连接) @param index 目标模型索引 */
    void animateTo(const QModelIndex& index) { moveToIndex(index); }

    /** @brief 动画滑动到指定索引位置(250ms OutCubic)，若动画运行中从当前位置衔接 @param index 目标模型索引 */
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

    /** @brief 无动画跳转到指定索引位置，用于初始化和恢复会话 @param index 目标模型索引 */
    void jumpToIndex(const QModelIndex& index)
    {
        QRect visualRect = m_navTree->visualRect(index);
        if (visualRect.height() <= 0) return;

        m_indicatorHeight = visualRect.height();
        m_indicatorY = visualRect.y();
        ++m_totalPositionChanges;
        update();
    }

    /** @brief 获取指示线Y坐标(QPropertyAnimation读访问器) @return 当前指示线顶部Y坐标 */
    qreal indicatorY() const { return m_indicatorY; }

    // ── 统计计数器 ──

    /** @brief 获取动画启动总次数 @return 累计动画启动次数 */
    quint64 totalAnimations() const { return m_totalAnimations; }
    /** @brief 获取位置变更总次数 @return 累计位置变更次数 */
    quint64 totalPositionChanges() const { return m_totalPositionChanges; }
    /** @brief 重置所有统计计数器为零 */
    void resetIndicatorStatistics() { m_totalAnimations = 0; m_totalPositionChanges = 0; }

    /** @brief 设置指示线Y坐标(QPropertyAnimation写访问器)，值变化时触发重绘 @param y 目标Y坐标 */
    void setIndicatorY(qreal y)
    {
        if (qFuzzyCompare(m_indicatorY, y)) return;
        m_indicatorY = y;
        emit indicatorYChanged(y);
        update();
    }

signals:
    /** @brief 指示线Y坐标变化信号 @param y 新的Y坐标值 */
    void indicatorYChanged(qreal y);

public slots:
    /** @brief 主题切换时刷新指示线颜色，触发paintEvent从ThemeManager重新取色 */
    void updateThemeColor() { update(); }

protected:
    /** @brief 绘制指示线，左侧3px宽accent色竖线，上下圆角 @param event 绘制事件(未使用) */
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

    /** @brief 事件过滤器，监听navTree的resize事件同步调整自身大小 @param watched 被监听的对象 @param event 事件对象 @return 是否消费事件 */
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
};

#endif // NAV_INDICATOR_WIDGET_H
