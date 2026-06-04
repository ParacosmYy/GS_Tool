/**
 * @file DeviceProfilePanel.cpp
 * @brief 设备配置选择面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "core/device/DeviceProfilePanel.h"

/** @brief 构造设备配置选择面板，初始化所有UI控件和信号连接 @param parent 父控件指针 */
DeviceProfilePanel::DeviceProfilePanel(QWidget *parent)
    : QWidget(parent)
    , m_profileList(new QListWidget(this))
    , m_newBtn(new QPushButton(tr("新建"), this))
    , m_editBtn(new QPushButton(tr("编辑"), this))
    , m_deleteBtn(new QPushButton(tr("删除"), this))
    , m_importBtn(new QPushButton(tr("导入"), this))
    , m_exportBtn(new QPushButton(tr("导出"), this))
{
    setObjectName(QStringLiteral("DeviceProfilePanel"));

    m_profileList->setObjectName("deviceProfileList");
    m_newBtn->setObjectName("deviceNewBtn");
    m_editBtn->setObjectName("deviceEditBtn");
    m_deleteBtn->setObjectName("deviceDeleteBtn");
    m_importBtn->setObjectName("deviceImportBtn");
    m_exportBtn->setObjectName("deviceExportBtn");

    auto *mainLayout = new QVBoxLayout(this);

    // 按钮行1：CRUD操作
    auto *crudLayout = new QHBoxLayout();
    crudLayout->addWidget(m_newBtn);
    crudLayout->addWidget(m_editBtn);
    crudLayout->addWidget(m_deleteBtn);
    crudLayout->addStretch();

    // 按钮行2：导入导出
    auto *ioLayout = new QHBoxLayout();
    ioLayout->addWidget(m_importBtn);
    ioLayout->addWidget(m_exportBtn);
    ioLayout->addStretch();

    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    // 设备列表
    m_profileList->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addLayout(crudLayout);
    mainLayout->addLayout(ioLayout);
    mainLayout->addWidget(m_profileList);

    // 连接信号
    connect(m_profileList, &QListWidget::itemSelectionChanged,
            this, &DeviceProfilePanel::onSelectionChanged);
    connect(m_profileList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem *item) {
        if (item && m_registry) {
            emit profileSelected(
                m_registry->findProfile(item->text()));
        }
    });
    connect(m_newBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onNewProfile);
    connect(m_editBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onEditProfile);
    connect(m_deleteBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onDeleteProfile);
    connect(m_importBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onImportProfiles);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &DeviceProfilePanel::onExportProfiles);
}

/**
 * @brief 设置关联的设备注册表并刷新列表
 * @param registry 设备注册表指针
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

/** @brief 获取当前选中的设备配置 @return 选中的设备配置，无选中时返回空配置 */
DeviceProfile DeviceProfilePanel::selectedProfile() const
{
    if (!m_registry) { return {}; }

    auto items = m_profileList->selectedItems();
    if (items.isEmpty()) { return {}; }

    return m_registry->findProfile(items.first()->text());
}

// CRUD操作/导入导出/列表刷新(onSelectionChanged/onNewProfile/onEditProfile/
// onDeleteProfile/onImportProfiles/onExportProfiles/refreshList)
// 见 DeviceProfilePanelActions.cpp
