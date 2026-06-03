/**
 * @file ToastWidget.h
 * @brief 通知吐司组件 -- 在父窗口右下角显示临时通知
 * 支持三种语义类型: Success(绿)/Error(红)/Info(强调色)
 * 动画: 弹出 300ms OutBack, 消失 250ms InCubic (CLAUDE.md §6.5)
 * 颜色全部从 ThemeManager 获取, 无硬编码
 */

#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QFontMetrics>
#include <QMap>
#include <QHash>
#include <QList>
#include <QElapsedTimer>
#include <QtGlobal>
#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

/** @brief 通知吐司 -- 临时弹出通知, 自动消失, 多条自动垂直堆叠 */
class ToastWidget : public QWidget {
    Q_OBJECT

public:
    enum class ToastType { Success, Error, Info }; ///< 通知类型

    /** @brief 显示吐司通知，弹出动画后自动定时消失 @param parent 父窗口 @param msg 消息文本 @param type 通知类型(Success/Error/Info) @param ms 自动消失延迟(毫秒) */
    static void show(QWidget* parent, const QString& msg,
                     ToastType type = ToastType::Info, int ms = 3000)
    {
        if (!parent) return;
        auto* toast = new ToastWidget(parent, msg, type);
        ++s_totalShows;
        if (type == ToastType::Error) ++s_totalErrors;
        activeToasts(parent).append(toast);
        if (!parent->property("_toastDestroyConnected").toBool()) {
            connect(parent, &QObject::destroyed, parent, [parent]() { activeToastsMap().remove(parent); });
            parent->setProperty("_toastDestroyConnected", true);
        }
        const auto& list = activeToasts(parent);
        int bottomY = parent->height() - kMargin;
        for (auto* t : list) {
            bottomY -= t->height();
            if (t != toast) bottomY -= kGap;
        }
        QPoint target(parent->width() - kMargin - toast->width(), bottomY);
        toast->move(target.x(), target.y() + 30);
        toast->QWidget::show();
        auto* slide = new QPropertyAnimation(toast, "pos");
        slide->setEndValue(target); slide->setDuration(Animations::kToastPopMs); slide->setEasingCurve(QEasingCurve::OutBack);
        auto* fade = new QPropertyAnimation(toast->m_opacityEffect, "opacity");
        fade->setEndValue(1.0); fade->setDuration(Animations::kToastPopMs); fade->setEasingCurve(QEasingCurve::OutBack);
        connect(slide, &QAbstractAnimation::finished, toast, [toast, ms]() {
            QTimer::singleShot(ms, toast, [toast]() { toast->dismiss(); });
        });
        slide->start(QAbstractAnimation::DeleteWhenStopped);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    }

    /** @brief 防抖吐司，在冷却期内重复调用同一消息将被忽略 @param parent 父窗口 @param msg 消息文本 @param type 通知类型(Success/Error/Info) @param cooldownMs 冷却间隔(毫秒) */
    static void showDebounced(QWidget* parent, const QString& msg,
                              ToastType type = ToastType::Info, int cooldownMs = 2000)
    {
        QString key = QString::number(static_cast<int>(type)) + "|" + msg;
        auto& map = debounceMap();
        if (map.contains(key) && map[key].elapsed() < cooldownMs) return;
        map[key].start();
        show(parent, msg, type);
    }

protected:
    /** @brief 自绘事件，绘制圆角背景、左侧语义色条、图标和消息文本 @param event 绘制事件(未使用) */
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        auto& theme = ThemeManager::instance();
        QColor bg = theme.color(ThemeManager::SemanticColor::BgSecondary);
        QColor accent = semanticColor();
        QColor txt = theme.color(ThemeManager::SemanticColor::TextPrimary);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(1, 1, -1, -1), kRadius, kRadius);
        p.fillPath(path, bg); p.save(); p.setClipPath(path);
        p.fillRect(QRect(0, 0, kLeftBorder, height()), accent); p.restore();
        int iconX = kLeftBorder + kPad;
        p.setPen(accent);
        QFont iconFont; iconFont.setFamilies({"Segoe UI Emoji", "Noto Color Emoji", "Apple Color Emoji"});
        iconFont.setPointSize(kIconSize); iconFont.setBold(true); p.setFont(iconFont);
        p.drawText(QRect(iconX, 0, kIconArea, height()), Qt::AlignCenter, iconChar());
        int textX = iconX + kIconArea + kPad;
        p.setPen(txt);
        QFont textFont; textFont.setFamilies({"Microsoft YaHei UI", "Segoe UI", "Noto Sans CJK SC"});
        textFont.setPointSize(12); p.setFont(textFont);
        p.drawText(QRect(textX, kPad, width() - textX - kPad, height() - kPad * 2),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, m_message);
    }

private:
    /** @brief 私有构造函数，初始化吐司控件的外观和尺寸 @param parent 父窗口 @param message 消息文本 @param type 通知类型 */
    explicit ToastWidget(QWidget* parent, const QString& message, ToastType type)
        : QWidget(parent), m_type(type), m_message(message)
    {
        setObjectName("toastWidget");
        setProperty("type", type == ToastType::Success ? "success"
                     : type == ToastType::Error ? "error" : "info");
        setAttribute(Qt::WA_TranslucentBackground); setFixedWidth(kWidth);
        QFont textFont; textFont.setFamilies({"Microsoft YaHei UI", "Segoe UI", "Noto Sans CJK SC"});
        textFont.setPointSize(12); QFontMetrics fm(textFont);
        int textW = kWidth - kLeftBorder - kPad * 3 - kIconArea;
        QRect bound = fm.boundingRect(0, 0, textW, 0, Qt::TextWordWrap, message);
        setFixedHeight(qMax(kMinHeight, bound.height() + kPad * 2));
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        m_opacityEffect->setOpacity(0.0); setGraphicsEffect(m_opacityEffect);
    }

    /** @brief 获取当前通知类型对应的语义颜色(ThemeManager) @return 语义颜色值 */
    QColor semanticColor() const {
        using SC = ThemeManager::SemanticColor;
        switch (m_type) {
        case ToastType::Success: return ThemeManager::instance().color(SC::Success);
        case ToastType::Error:   return ThemeManager::instance().color(SC::Error);
        case ToastType::Info:    return ThemeManager::instance().color(SC::Accent);
        }
        return ThemeManager::instance().color(SC::Accent);
    }

    /** @brief 获取当前通知类型对应的图标Unicode字符 @return 图标字符字符串 */
    QString iconChar() const {
        switch (m_type) {
        case ToastType::Success: return tr("✓");
        case ToastType::Error:   return tr("✕");
        case ToastType::Info:    return tr("ℹ");
        }
        return tr("ℹ");
    }

    /** @brief 消失动画，InCubic缓动，向上飘出30px并淡出(并行动画组) */
    void dismiss() {
        ++s_totalDismisses;
        auto* group = new QParallelAnimationGroup(this);
        auto* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
        fadeOut->setEndValue(0.0); fadeOut->setDuration(Animations::kToastDismissMs);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);
        group->addAnimation(fadeOut);
        auto* drift = new QPropertyAnimation(this, "pos");
        drift->setStartValue(pos()); drift->setEndValue(pos() + QPoint(0, -30));
        drift->setDuration(Animations::kToastDismissMs); drift->setEasingCurve(QEasingCurve::InCubic);
        group->addAnimation(drift);
        connect(group, &QAbstractAnimation::finished, this, [this]() {
            QWidget* pw = parentWidget();
            if (pw) { activeToasts(pw).removeOne(this); repositionToasts(pw); }
            deleteLater();
        });
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }

    /** @brief 获取指定父窗口的活跃吐司列表引用 @param parent 父窗口 @return 活跃吐司列表的引用 */
    static QList<ToastWidget*>& activeToasts(QWidget* parent) { return activeToastsMap()[parent]; }

    /** @brief 获取全局父窗口-吐司列表映射表(单例) @return 映射表引用 */
    static QMap<QWidget*, QList<ToastWidget*>>& activeToastsMap() {
        static QMap<QWidget*, QList<ToastWidget*>> map; return map; }

    /** @brief 获取全局防抖计时器映射表(单例) @return 防抖计时器映射引用 */
    static QHash<QString, QElapsedTimer>& debounceMap() {
        static QHash<QString, QElapsedTimer> map; return map; }

    /** @brief 吐司消失后重新排列剩余活跃吐司的垂直位置 @param parent 父窗口 */
    static void repositionToasts(QWidget* parent) {
        if (!parent) return;
        auto& list = activeToasts(parent);
        int bottomY = parent->height() - kMargin;
        for (auto* t : list) {
            int y = bottomY - t->height();
            QPoint target(parent->width() - kMargin - t->width(), y);
            if (t->pos() != target) {
                auto* slide = new QPropertyAnimation(t, "pos");
                slide->setEndValue(target); slide->setDuration(Animations::kNavIndicatorMs);
                slide->setEasingCurve(QEasingCurve::OutCubic);
                slide->start(QAbstractAnimation::DeleteWhenStopped);
            }
            bottomY = y - kGap;
        }
    }

    ToastType m_type;                                  ///< 通知类型
    QString m_message;                                 ///< 消息文本
    QGraphicsOpacityEffect* m_opacityEffect = nullptr; ///< 淡入淡出特效

    // ---- 统计计数器(静态，跨所有实例累积) ----
    static inline quint64 s_totalShows = 0;          ///< 总显示次数
    static inline quint64 s_totalDismisses = 0;      ///< 总消失次数
    static inline quint64 s_totalErrors = 0;         ///< 总错误通知次数
public:
    /** @brief 获取吐司总显示次数 @return 累计显示次数 */
    static quint64 totalShows() { return s_totalShows; }
    /** @brief 获取吐司总消失次数 @return 累计消失次数 */
    static quint64 totalDismisses() { return s_totalDismisses; }
    /** @brief 获取错误通知总次数 @return 累计错误通知次数 */
    static quint64 totalErrors() { return s_totalErrors; }
    /** @brief 重置吐司统计计数器 */
    static void resetToastStatistics() { s_totalShows = 0; s_totalDismisses = 0; s_totalErrors = 0; }
private:
    static constexpr int kWidth = 320, kMinHeight = 48;
    static constexpr int kMargin = 16, kRadius = 8, kGap = 8;
    static constexpr int kLeftBorder = 4, kPad = 12, kIconArea = 24, kIconSize = 14;
};

#endif // TOASTWIDGET_H
