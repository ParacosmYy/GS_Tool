/** @file ToastWidget.h @brief 通知吐司组件 -- 右下角临时通知，Success(绿)/Error(红)/Info(强调色)，弹出300ms OutBack/消失250ms InCubic */
#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QElapsedTimer>
#include <QMap>
#include <QHash>
#include <QList>

/** @brief 通知吐司 -- 临时弹出通知, 自动消失, 多条自动垂直堆叠 */
class ToastWidget : public QWidget {
    Q_OBJECT
public:
    enum class ToastType { Success, Error, Info }; ///< 通知类型枚举
    static void show(QWidget* parent, const QString& msg,
                     ToastType type = ToastType::Info, int ms = 3000);
    static void showDebounced(QWidget* parent, const QString& msg,
                              ToastType type = ToastType::Info, int cooldownMs = 2000);
    static quint64 totalShows();            ///< 获取总显示次数
    static quint64 totalDismisses();        ///< 获取总消失次数
    static quint64 totalErrors();           ///< 获取总错误通知次数
    static void resetToastStatistics();     ///< 重置统计

protected:
    void paintEvent(QPaintEvent*) override; ///< 自绘: 圆角背景+左侧语义色条+图标+消息

private:
    explicit ToastWidget(QWidget* parent, const QString& message, ToastType type);
    QColor semanticColor() const;           ///< 当前通知类型的语义颜色
    QString iconChar() const;               ///< 通知类型对应图标Unicode字符
    void dismiss();                         ///< 消失动画: InCubic缓动, 向上飘出30px并淡出
    static QList<ToastWidget*>& activeToasts(QWidget* parent);
    static QMap<QWidget*, QList<ToastWidget*>>& activeToastsMap();
    static QHash<QString, QElapsedTimer>& debounceMap();
    static void repositionToasts(QWidget* parent);

    ToastType m_type;                                  ///< 通知类型
    QString m_message;                                 ///< 消息文本
    QGraphicsOpacityEffect* m_opacityEffect = nullptr; ///< 淡入淡出特效
    static inline quint64 s_totalShows = 0;     ///< 总显示次数
    static inline quint64 s_totalDismisses = 0; ///< 总消失次数
    static inline quint64 s_totalErrors = 0;    ///< 总错误通知次数
    static constexpr int kWidth = 320, kMinHeight = 48, kMargin = 16, kRadius = 8;
    static constexpr int kGap = 8, kLeftBorder = 4, kPad = 12, kIconArea = 24, kIconSize = 14;
};
#endif // TOASTWIDGET_H
