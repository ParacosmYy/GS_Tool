/**
 * @file ToastWidget2.cpp
 * @brief Toast通知组件v2实现 — 自动消失的悬浮提示(成功/警告/错误/信息)
 */
#include "widgets/toast/ToastWidget2.h"
#include "core/theme/ThemeManager.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>

/** @brief 构造函数，创建无边框透明窗口+自动关闭定时器 @param parent 父Widget */
ToastWidget::ToastWidget(QWidget *parent) : QWidget(parent), m_timer(new QTimer(this)) {
    setObjectName("ToastWidget2");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUi();
    connect(m_timer, &QTimer::timeout, this, &ToastWidget::dismiss);
    hide();
}
/** @brief 析构函数 */
ToastWidget::~ToastWidget() = default;

/** @brief 初始化UI — 圆角标签+固定尺寸 */
void ToastWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    m_label = new QLabel(this);
    m_label->setObjectName("toastLabel");
    m_label->setWordWrap(true);
    layout->addWidget(m_label);
    setFixedSize(300, 60);
}

/** @brief 显示Toast消息 @param text 消息文本 @param type Toast类型 @param ms 显示时长(毫秒) */
void ToastWidget::showMessage(const QString &text, ToastType type, int ms) {
    ++m_totalMessages;
    switch (type) {
    case Info: ++m_totalInfo; break;
    case Success: ++m_totalSuccess; break;
    case Warning: ++m_totalWarning; break;
    case Error: ++m_totalError; break;
    }
    m_type = type; m_label->setText(text); updateStyle();
    m_timer->start(ms); show(); raise();
}

/** @brief 显示成功提示 @param t 消息文本 @param ms 显示时长 */
void ToastWidget::showSuccess(const QString &t, int ms) { showMessage(t, Success, ms); }
/** @brief 显示警告提示 @param t 消息文本 @param ms 显示时长 */
void ToastWidget::showWarning(const QString &t, int ms) { showMessage(t, Warning, ms); }
/** @brief 显示错误提示 @param t 消息文本 @param ms 显示时长 */
void ToastWidget::showError(const QString &t, int ms) { showMessage(t, Error, ms); }

/** @brief 关闭Toast通知 */
void ToastWidget::dismiss() { ++m_totalDismisses; m_timer->stop(); hide(); }
/** @brief 查询Toast是否可见 @return 可见返回true */
bool ToastWidget::isVisible() const { return QWidget::isVisible(); }
/** @brief 设置Toast显示位置 @param c 屏幕角落 */
void ToastWidget::setPosition(Qt::Corner c) { m_corner = c; }

/** @brief 绘制Toast背景 — 使用ThemeManager语义色的圆角矩形 */
void ToastWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    QColor bg;
    switch (m_type) {
    case Success: bg = ThemeManager::instance().color(ThemeManager::SemanticColor::Success); break;
    case Warning: bg = ThemeManager::instance().color(ThemeManager::SemanticColor::Warning); break;
    case Error: bg = ThemeManager::instance().color(ThemeManager::SemanticColor::Error); break;
    case Info: default: bg = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent); break;
    }
    bg.setAlpha(220);
    p.setBrush(bg); p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 8, 8);
}

/** @brief 根据消息级别更新标签样式 — 按类型设置QSS类名 */
void ToastWidget::updateStyle() {
    QString styleClass;
    switch (m_type) {
    case Success: styleClass = "toast-success"; break;
    case Warning: styleClass = "toast-warning"; break;
    case Error:   styleClass = "toast-error";   break;
    case Info: default: styleClass = "toast-info"; break;
    }
    m_label->setProperty("class", styleClass);
    style()->unpolish(m_label);
    style()->polish(m_label);
}

/** @brief 鼠标点击关闭Toast @param event 鼠标事件(未使用) */
void ToastWidget::mousePressEvent(QMouseEvent *) { dismiss(); }
