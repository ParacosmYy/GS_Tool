/**
 * @file ToastWidget.h
 * @brief 通知吐司组件 — 在父窗口右下角显示临时通知
 *
 * 支持三种语义类型: Success(绿)/Error(红)/Info(强调色)
 * 动画: 弹出 300ms OutBack, 消失 250ms InCubic (CLAUDE.md §6.5)
 * 颜色全部从 ThemeManager 获取, 无硬编码
 *
 * 使用: ToastWidget::show(this, tr("连接成功"), ToastWidget::ToastType::Success);
 */

#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QFontMetrics>
#include <QMap>
#include <QList>
#include "core/ThemeManager.h"

/** @brief 通知吐司 — 临时弹出通知, 自动消失, 多条自动垂直堆叠 */
class ToastWidget : public QWidget {
    Q_OBJECT

public:
    enum class ToastType { Success, Error, Info }; ///< 通知类型

    /** @brief 显示吐司(唯一公开接口), 在 parent 右下角创建并显示 */
    static void show(QWidget* parent, const QString& message,
                     ToastType type = ToastType::Info, int durationMs = 3000)
    {
        if (!parent) return;
        auto* toast = new ToastWidget(parent, message, type);
        activeToasts(parent).append(toast);

        // 父控件销毁时清理静态 map 中的悬挂指针条目
        connect(parent, &QObject::destroyed, parent, [parent]() {
            activeToastsMap().remove(parent);
        });

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
        slide->setEndValue(target);
        slide->setDuration(300);
        slide->setEasingCurve(QEasingCurve::OutBack);

        auto* fade = new QPropertyAnimation(toast->m_opacityEffect, "opacity");
        fade->setEndValue(1.0);
        fade->setDuration(300);
        fade->setEasingCurve(QEasingCurve::OutBack);

        connect(slide, &QAbstractAnimation::finished, toast, [toast, durationMs]() {
            QTimer::singleShot(durationMs, toast, [toast]() { toast->dismiss(); });
        });

        slide->start(QAbstractAnimation::DeleteWhenStopped);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        auto& theme = ThemeManager::instance();
        QColor bg = theme.color(ThemeManager::SemanticColor::BgSecondary);
        QColor accent = semanticColor();
        QColor txt = theme.color(ThemeManager::SemanticColor::TextPrimary);

        QPainterPath path;
        path.addRoundedRect(rect().adjusted(1, 1, -1, -1), kRadius, kRadius);
        p.fillPath(path, bg);

        p.save();
        p.setClipPath(path);
        p.fillRect(QRect(0, 0, kLeftBorder, height()), accent);
        p.restore();

        int iconX = kLeftBorder + kPad;
        p.setPen(accent);
        p.setFont(QFont("Segoe UI Emoji", kIconSize, QFont::Bold));
        p.drawText(QRect(iconX, 0, kIconArea, height()), Qt::AlignCenter, iconChar());

        int textX = iconX + kIconArea + kPad;
        p.setPen(txt);
        p.setFont(QFont("Microsoft YaHei UI", 12));
        p.drawText(QRect(textX, kPad, width() - textX - kPad, height() - kPad * 2),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, m_message);
    }

private:
    explicit ToastWidget(QWidget* parent, const QString& message, ToastType type)
        : QWidget(parent), m_type(type), m_message(message)
    {
        setObjectName("toastWidget");
        setProperty("type", type == ToastType::Success ? "success"
                     : type == ToastType::Error   ? "error"
                                                  : "info");
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedWidth(kWidth);
        QFont font("Microsoft YaHei UI", 12);
        QFontMetrics fm(font);
        int textW = kWidth - kLeftBorder - kPad * 3 - kIconArea;
        QRect bound = fm.boundingRect(0, 0, textW, 0, Qt::TextWordWrap, message);
        setFixedHeight(qMax(kMinHeight, bound.height() + kPad * 2));
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        m_opacityEffect->setOpacity(0.0);
        setGraphicsEffect(m_opacityEffect);
    }

    QColor semanticColor() const {                          ///< 获取语义色(ThemeManager)
        using SC = ThemeManager::SemanticColor;
        switch (m_type) {
        case ToastType::Success: return ThemeManager::instance().color(SC::Success);
        case ToastType::Error:   return ThemeManager::instance().color(SC::Error);
        case ToastType::Info:    return ThemeManager::instance().color(SC::Accent);
        }
        return ThemeManager::instance().color(SC::Accent);
    }

    QString iconChar() const {                               ///< 图标Unicode字符
        switch (m_type) {
        case ToastType::Success: return QStringLiteral("\u2713");
        case ToastType::Error:   return QStringLiteral("\u2715");
        case ToastType::Info:    return QStringLiteral("\u2139");
        }
        return QStringLiteral("\u2139");
    }

    void dismiss() {                                         ///< 250ms InCubic消失动画
        auto* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
        fadeOut->setEndValue(0.0);
        fadeOut->setDuration(250);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);
        connect(fadeOut, &QAbstractAnimation::finished, this, [this]() {
            QWidget* pw = parentWidget();
            if (pw) { activeToasts(pw).removeOne(this); repositionToasts(pw); }
            deleteLater();
        });
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    }

    static QList<ToastWidget*>& activeToasts(QWidget* parent) { ///< 活跃吐司列表
        return activeToastsMap()[parent];
    }

    static QMap<QWidget*, QList<ToastWidget*>>& activeToastsMap() { ///< 活跃吐司静态映射
        static QMap<QWidget*, QList<ToastWidget*>> map;
        return map;
    }

    static void repositionToasts(QWidget* parent) {              ///< 消失后重排位置
        if (!parent) return;
        auto& list = activeToasts(parent);
        int bottomY = parent->height() - kMargin;
        for (auto* t : list) {
            int y = bottomY - t->height();
            QPoint target(parent->width() - kMargin - t->width(), y);
            if (t->pos() != target) {
                auto* slide = new QPropertyAnimation(t, "pos");
                slide->setEndValue(target);
                slide->setDuration(200);
                slide->setEasingCurve(QEasingCurve::OutCubic);
                slide->start(QAbstractAnimation::DeleteWhenStopped);
            }
            bottomY = y - kGap;
        }
    }

    ToastType m_type;                                  ///< 通知类型
    QString m_message;                                 ///< 消息文本
    QGraphicsOpacityEffect* m_opacityEffect = nullptr; ///< 淡入淡出特效

    static constexpr int kWidth = 320;    ///< 吐司固定宽度
    static constexpr int kMinHeight = 48; ///< 最小高度
    static constexpr int kMargin = 16;    ///< 与窗口边缘间距
    static constexpr int kRadius = 8;     ///< 圆角半径
    static constexpr int kLeftBorder = 4; ///< 左侧彩色边框宽度
    static constexpr int kPad = 12;       ///< 内边距
    static constexpr int kIconArea = 24;  ///< 图标区域宽度
    static constexpr int kIconSize = 14;  ///< 图标字号
    static constexpr int kGap = 8;        ///< 吐司间距
};

#endif // TOASTWIDGET_H
