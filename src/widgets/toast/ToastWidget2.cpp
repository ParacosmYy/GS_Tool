#include "widgets/toast/ToastWidget2.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>

ToastWidget::ToastWidget(QWidget *parent) : QWidget(parent), m_timer(new QTimer(this)) {
    setObjectName("ToastWidget2");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUi();
    connect(m_timer, &QTimer::timeout, this, &ToastWidget::dismiss);
    hide();
}
ToastWidget::~ToastWidget() = default;

void ToastWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    m_label = new QLabel(this);
    m_label->setObjectName("toastLabel");
    m_label->setWordWrap(true);
    layout->addWidget(m_label);
    setFixedSize(300, 60);
}

void ToastWidget::showMessage(const QString &text, ToastType type, int ms) {
    m_type = type; m_label->setText(text); updateStyle();
    m_timer->start(ms); show(); raise();
}

void ToastWidget::showSuccess(const QString &t, int ms) { showMessage(t, Success, ms); }
void ToastWidget::showWarning(const QString &t, int ms) { showMessage(t, Warning, ms); }
void ToastWidget::showError(const QString &t, int ms) { showMessage(t, Error, ms); }

void ToastWidget::dismiss() { m_timer->stop(); hide(); }
bool ToastWidget::isVisible() const { return QWidget::isVisible(); }
void ToastWidget::setPosition(Qt::Corner c) { m_corner = c; }

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

void ToastWidget::mousePressEvent(QMouseEvent *) { dismiss(); }
