#include "BackgroundSettingsPopup.h"
#include "BackgroundWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

BackgroundSettingsPopup::BackgroundSettingsPopup(BackgroundWidget* bgWidget, QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_bgWidget(bgWidget)
{
    setObjectName("bgSettingsPopup");
    setFixedSize(260, 220);
    setAttribute(Qt::WA_TranslucentBackground, false);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ---- 模糊半径 ----
    auto* blurLayout = new QHBoxLayout;
    auto* blurLbl = new QLabel(tr("磨砂模糊:"), this);
    blurLbl->setFixedWidth(70);
    m_blurSlider = new QSlider(Qt::Horizontal, this);
    m_blurSlider->setObjectName("bgBlurSlider");
    m_blurSlider->setRange(0, 30);
    m_blurSlider->setValue(int(m_bgWidget->blurRadius()));
    m_blurValueLbl = new QLabel(QString::number(int(m_bgWidget->blurRadius())), this);
    m_blurValueLbl->setFixedWidth(28);
    blurLayout->addWidget(blurLbl);
    blurLayout->addWidget(m_blurSlider, 1);
    blurLayout->addWidget(m_blurValueLbl);
    mainLayout->addLayout(blurLayout);

    connect(m_blurSlider, &QSlider::valueChanged, this, [this](int val) {
        m_bgWidget->setBlurRadius(val);
        m_blurValueLbl->setText(QString::number(val));
    });

    // ---- 透明度 ----
    auto* opacityLayout = new QHBoxLayout;
    auto* opacityLbl = new QLabel(tr("背景透明度:"), this);
    opacityLbl->setFixedWidth(70);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setObjectName("bgOpacitySlider");
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(int(m_bgWidget->bgOpacity() * 100));
    m_opacityValueLbl = new QLabel(QString::number(int(m_bgWidget->bgOpacity() * 100)) + "%", this);
    m_opacityValueLbl->setFixedWidth(36);
    opacityLayout->addWidget(opacityLbl);
    opacityLayout->addWidget(m_opacitySlider, 1);
    opacityLayout->addWidget(m_opacityValueLbl);
    mainLayout->addLayout(opacityLayout);

    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int val) {
        m_bgWidget->setBgOpacity(val / 100.0);
        m_opacityValueLbl->setText(QString::number(val) + "%");
    });

    // ---- 涟漪特效开关 ----
    auto* rippleCheck = new QCheckBox(tr("点击涟漪特效"), this);
    rippleCheck->setChecked(m_bgWidget->rippleEnabled());
    mainLayout->addWidget(rippleCheck);

    connect(rippleCheck, &QCheckBox::toggled, m_bgWidget, &BackgroundWidget::setRippleEnabled);
}

void BackgroundSettingsPopup::syncFromWidget()
{
    m_blurSlider->setValue(int(m_bgWidget->blurRadius()));
    m_blurValueLbl->setText(QString::number(int(m_bgWidget->blurRadius())));
    m_opacitySlider->setValue(int(m_bgWidget->bgOpacity() * 100));
    m_opacityValueLbl->setText(QString::number(int(m_bgWidget->bgOpacity() * 100)) + "%");
}

void BackgroundSettingsPopup::hideEvent(QHideEvent* event)
{
    emit hidden();
    QWidget::hideEvent(event);
}
