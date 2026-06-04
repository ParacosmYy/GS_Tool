/** @file NavIndicatorWidget.cpp @brief NavIndicatorWidget实现 -- 构造/绘制/事件/动画/统计 */

#include <QPainter>
#include <QPaintEvent>
#include <QHeaderView>

#include "core/navigation/NavIndicatorWidget.h"
#include "core/theme/ThemeManager.h"
#include "shared/AnimationConstants.h"

NavIndicatorWidget::NavIndicatorWidget(QTreeView* navTree, QWidget* parent)
    : QWidget(navTree)
    , m_navTree(navTree)
    , m_indicatorY(0.0)
    , m_indicatorHeight(0)
{
    Q_UNUSED(parent)
    setObjectName("navIndicator");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    m_slideAnim = new QPropertyAnimation(this, "indicatorY", this);
    m_slideAnim->setDuration(Animations::kNavIndicatorMs);
    m_slideAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_navTree->installEventFilter(this);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &NavIndicatorWidget::updateThemeColor);
}

void NavIndicatorWidget::animateTo(const QModelIndex& index) { moveToIndex(index); }

void NavIndicatorWidget::moveToIndex(const QModelIndex& index)
{
    QRect visualRect = m_navTree->visualRect(index);
    qreal targetY = visualRect.y();
    int targetHeight = visualRect.height();
    if (targetHeight <= 0) return;

    m_indicatorHeight = targetHeight;
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

void NavIndicatorWidget::jumpToIndex(const QModelIndex& index)
{
    QRect visualRect = m_navTree->visualRect(index);
    if (visualRect.height() <= 0) return;
    m_indicatorHeight = visualRect.height();
    m_indicatorY = visualRect.y();
    ++m_totalPositionChanges;
    ++m_totalJumpMoves;
    update();
}

qreal NavIndicatorWidget::indicatorY() const { return m_indicatorY; }

void NavIndicatorWidget::setIndicatorY(qreal y)
{
    if (qFuzzyCompare(m_indicatorY, y)) return;
    m_indicatorY = y;
    emit indicatorYChanged(y);
    update();
}

// ── 统计 ──
quint64 NavIndicatorWidget::totalAnimations() const { return m_totalAnimations; }
quint64 NavIndicatorWidget::totalPositionChanges() const { return m_totalPositionChanges; }
quint64 NavIndicatorWidget::totalJumpMoves() const { return m_totalJumpMoves; }
quint64 NavIndicatorWidget::totalThemeUpdates() const { return m_totalThemeUpdates; }
quint64 NavIndicatorWidget::totalRepaints() const { return m_totalRepaints; }
quint64 NavIndicatorWidget::totalResizeSyncs() const { return m_totalResizeSyncs; }

void NavIndicatorWidget::resetIndicatorStatistics()
{
    m_totalAnimations = 0;
    m_totalPositionChanges = 0;
    m_totalJumpMoves = 0;
    m_totalThemeUpdates = 0;
    m_totalRepaints = 0;
    m_totalResizeSyncs = 0;
}

// ── 槽 ──
void NavIndicatorWidget::updateThemeColor() { ++m_totalThemeUpdates; update(); }

// ── 绘制 ──
void NavIndicatorWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    ++m_totalRepaints;
    if (m_indicatorHeight <= 0) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor accentColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);

    constexpr int kLineWidth = 3;
    constexpr int kRadius = 2;
    int headerHeight = m_navTree->header()->height();
    qreal drawY = m_indicatorY + headerHeight;
    QRectF indicatorRect(0, drawY, kLineWidth, m_indicatorHeight);

    painter.setPen(Qt::NoPen);
    painter.setBrush(accentColor);
    painter.drawRoundedRect(indicatorRect, kRadius, kRadius);
}

bool NavIndicatorWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_navTree && event->type() == QEvent::Resize) {
        ++m_totalResizeSyncs;
        setGeometry(0, 0, m_navTree->width(), m_navTree->height());
    }
    return QWidget::eventFilter(watched, event);
}
