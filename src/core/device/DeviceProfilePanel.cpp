/**
 * @file DeviceProfilePanel.cpp
 * @brief 设备配置选择面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "core/device/DeviceProfilePanel.h"

/**
 * @brief 构造函数，初始化设备选择面板布局
 */
DeviceProfilePanel::DeviceProfilePanel(QWidget *parent)
    : QWidget(parent)
    , m_profileList(new QListWidget(this))
    , m_newBtn(new QPushButton(tr("新建"), this))
    , m_editBtn(new QPushButton(tr("编辑"), this))
    , m_deleteBtn(new QPushButton(tr("删除"), this))
{
    setObjectName(QStringLiteral("DeviceProfilePanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 按钮行
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();

    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    // 设备列表
    m_profileList->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(m_profileList);

    // 连接信号
    connect(m_profileList, &QListWidget::itemSelectionChanged,
            this, &DeviceProfilePanel::onSelectionChanged);
    connect(m_newBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onNewProfile);
    connect(m_editBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onEditProfile);
    connect(m_deleteBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onDeleteProfile);
}

/**
 * @brief 设置关联的注册表并刷新列表
 */
void DeviceProfilePanel::setRegistry(DeviceRegistry *registry)
{
    m_registry = registry;

    if (m_registry) {
        connect(m_registry, &DeviceRegistry::profilesChanged,
                this, &DeviceProfilePanel::refreshList);
        refreshList();
    }
}

/**
 * @brief 获取当前选中的设备配置
 */
DeviceProfile DeviceProfilePanel::selectedProfile() const
{
    if (!m_registry) {
        return {};
    }

    auto items = m_profileList->selectedItems();
    if (items.isEmpty()) {
        return {};
    }

    return m_registry->findProfile(items.first()->text());
}

/**
 * @brief 处理列表选择变更
 */
void DeviceProfilePanel::onSelectionChanged()
{
    bool hasSelection = !m_profileList->selectedItems().isEmpty();
    m_editBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);

    if (hasSelection) {
        emit profileSelected(selectedProfile());
    }
}

/**
 * @brief 新建设备配置
 */
void DeviceProfilePanel::onNewProfile()
{
    emit newProfileRequested();
}

/**
 * @brief 编辑选中的配置（占位实现）
 */
void DeviceProfilePanel::onEditProfile()
{
    // TODO: 打开编辑对话框
}

/**
 * @brief 删除选中的配置
 */
void DeviceProfilePanel::onDeleteProfile()
{
    if (!m_registry) {
        return;
    }

    auto items = m_profileList->selectedItems();
    if (!items.isEmpty()) {
        m_registry->removeProfile(items.first()->text());
    }
}

/**
 * @brief 刷新设备列表
 */
void DeviceProfilePanel::refreshList()
{
    m_profileList->clear();

    if (!m_registry) {
        return;
    }

    for (const auto &profile : m_registry->profiles()) {
        m_profileList->addItem(profile.name);
    }
}
