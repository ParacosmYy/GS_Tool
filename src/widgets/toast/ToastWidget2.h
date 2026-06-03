#pragma once
#include <QWidget>
#include <QString>
#include <QTimer>
#include <QLabel>

class ToastWidget : public QWidget {
    Q_OBJECT
public:
    enum ToastType { Info, Success, Warning, Error };
    Q_ENUM(ToastType)

    explicit ToastWidget(QWidget *parent = nullptr);
    ~ToastWidget() override;
    void showMessage(const QString &text, ToastType type = Info, int durationMs = 3000);
    void showSuccess(const QString &text, int durationMs = 3000);
    void showWarning(const QString &text, int durationMs = 4000);
    void showError(const QString &text, int durationMs = 5000);
    void dismiss();
    bool isVisible() const;
    void setPosition(Qt::Corner corner);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    void setupUi();
    void updateStyle();
    QLabel *m_label = nullptr;
    QTimer *m_timer = nullptr;
    ToastType m_type = Info;
    Qt::Corner m_corner = Qt::TopRightCorner;
};
