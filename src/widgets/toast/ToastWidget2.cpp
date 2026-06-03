/**
 * @file ToastWidget2.cpp
 * @brief Toast通知组件v2实现 — 自动消失的悬浮提示(成功/警告/错误/信息)
 */
#include "widgets/toast/ToastWidget2.h"
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
void ToastWidget::dismiss() { m_timer->stop(); hide(); }
/** @brief 查询Toast是否可见 @return 可见返回true */
bool ToastWidget::isVisible() const { return QWidget::isVisible(); }
/** @brief 设置Toast显示位置 @param c 屏幕角落 */
void ToastWidget::setPosition(Qt::Corner c) { m_corner = c; }

/** @brief 绘制Toast背景 — 按类型着色的圆角矩形 */
void ToastWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    QColor bg;
    switch (m_type) {
    case Success: bg = QColor(46, 125, 50, 220); break;
    case Warning: bg = QColor(237, 162, 0, 220); break;
    case Error: bg = QColor(198, 40, 40, 220); break;
    case Info: default: bg = QColor(33, 150, 243, 220); break;
    }
    p.setBrush(bg); p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 8, 8);
}

/** @brief 鼠标点击关闭Toast @param event 鼠标事件(未使用) */
void ToastWidget::mousePressEvent(QMouseEvent *) { dismiss(); }
