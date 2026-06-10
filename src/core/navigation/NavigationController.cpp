/**
 * @file NavigationController.cpp
 * @brief 导航控制器实现 - 导航树构建、面板查找、面板状态管理
 *
 * 动画相关方法已拆分至 NavigationControllerAnimations.cpp:
 *   - animateSlideOut / animateSlideIn / switchToPanel (面板滑动切换)
 *   - startBreathingAnimation / stopBreathingAnimation (连接状态呼吸动画)
 *   - parentContainerWidth (动画辅助)
 */

#include "core/navigation/NavigationController.h"
#include "core/theme/ThemeManager.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>
#include <QStringList>
#include <QMap>

/** @brief 创建导航树连接类型指示圆点图标(8x8透明底+抗锯齿彩色圆点) @param color 圆点颜色 @return 图标实例 */
static QIcon createDotIcon(const QColor& color)
{
    QPixmap dot(8, 8);
    dot.fill(Qt::transparent);
    QPainter painter(&dot);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(1, 1, 6, 6);
    return QIcon(dot);
}

static constexpr int kNavPanelMappingIndexRole = Qt::UserRole + 1;

/** @brief 构造导航控制器，连接ThemeManager::themeChanged信号用于刷新图标颜色 @param parent 父对象 */
NavigationController::NavigationController(QObject* parent)
    : QObject(parent)
{
    // 主题切换时刷新导航树图标颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &NavigationController::onThemeChanged);
}

/** @brief 析构导航控制器，清理呼吸动画和进行中的切换动画防止资源泄漏 */
NavigationController::~NavigationController()
{
    // 清理进行中的面板切换动画
    if (m_switchAnimGroup) {
        m_switchAnimGroup->stop();
        // QParallelAnimationGroup 的子动画如果使用 DeleteWhenStopped
        // 在 group stop 时会被自动删除，这里只需删除 group 自身
        delete m_switchAnimGroup;
        m_switchAnimGroup = nullptr;
    }
    stopBreathingAnimation(nullptr);
}

/** @brief 构建导航树模型并展开全部节点，按category字段自动分组并使用Accent色圆点图标 @param navTree 导航树视图控件 @param mappings 面板映射表(必须包含category字段) */
void NavigationController::buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings)
{
    if (m_navTreeClickedConnection) {
        disconnect(m_navTreeClickedConnection);
        m_navTreeClickedConnection = {};
    }

    m_navTree = navTree;
    m_navPanelMappings = mappings;

    auto* treeModel = new QStandardItemModel(this);
    auto* rootItem = treeModel->invisibleRootItem();

    // ---- 数据驱动构建: 按 category 分组 ----
    // 遍历映射表，收集有序且去重的 category 列表
    QStringList categories;
    for (const auto& mapping : mappings) {
        QString cat = QCoreApplication::translate("Nav", mapping.category);
        if (!categories.contains(cat)) {
            categories.append(cat);
        }
    }

    // 每个 category 创建一个分组节点
    QColor dotColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
    QMap<QString, QStandardItem*> categoryItems;

    for (const QString& cat : categories) {
        auto* catItem = new QStandardItem(createDotIcon(dotColor), cat);
        catItem->setEditable(false);
        rootItem->appendRow(catItem);
        categoryItems[cat] = catItem;
    }

    // 将面板叶子节点添加到对应的 category 分组
    for (int mappingIndex = 0; mappingIndex < mappings.size(); ++mappingIndex) {
        const auto& mapping = mappings[mappingIndex];
        QString cat = QCoreApplication::translate("Nav", mapping.category);
        auto* catItem = categoryItems.value(cat, nullptr);
        if (!catItem) continue;

        QString panelName = QCoreApplication::translate("MainWindow", mapping.name);
        auto* panelItem = new QStandardItem(panelName);
        panelItem->setEditable(false);
        panelItem->setData(mappingIndex, kNavPanelMappingIndexRole);
        catItem->appendRow(panelItem);
    }

    navTree->setModel(treeModel);
    navTree->expandAll();  // 默认展开所有分组
    m_navTreeClickedConnection = connect(navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        bool ok = false;
        const int mappingIndex = index.data(kNavPanelMappingIndexRole).toInt(&ok);
        if (!ok || mappingIndex < 0 || mappingIndex >= m_navPanelMappings.size()) {
            return;
        }

        QWidget* panel = m_navPanelMappings[mappingIndex].widget;
        if (panel) {
            switchToPanel(panel);
        }
    });
    ++m_totalTreeExpansions;  ///< 统计: 导航树展开操作递增
    ++m_totalNavTreeRebuilds; ///< 统计: 导航树重建次数递增
}

/** @brief 根据目标面板同步导航树当前选中项 */
void NavigationController::syncNavTreeSelection(QWidget* panel)
{
    if (!m_navTree || !panel) {
        return;
    }

    auto* model = qobject_cast<QStandardItemModel*>(m_navTree->model());
    if (!model) {
        return;
    }

    auto* root = model->invisibleRootItem();
    if (!root) {
        return;
    }

    for (int mappingIndex = 0; mappingIndex < m_navPanelMappings.size(); ++mappingIndex) {
        if (m_navPanelMappings[mappingIndex].widget != panel) {
            continue;
        }

        for (int categoryRow = 0; categoryRow < root->rowCount(); ++categoryRow) {
            auto* categoryItem = root->child(categoryRow);
            if (!categoryItem) {
                continue;
            }

            for (int panelRow = 0; panelRow < categoryItem->rowCount(); ++panelRow) {
                auto* panelItem = categoryItem->child(panelRow);
                if (!panelItem || panelItem->data(kNavPanelMappingIndexRole).toInt() != mappingIndex) {
                    continue;
                }

                const QModelIndex panelModelIndex = panelItem->index();
                m_navTree->setCurrentIndex(panelModelIndex);
                m_navTree->scrollTo(panelModelIndex);
                return;
            }
        }
        return;
    }
}

/** @brief 收集所有可切换面板widget(从映射表中提取所有非空widget) @return 面板widget向量 */
QVector<QWidget*> NavigationController::allSwitchablePanels() const
{
    QVector<QWidget*> panels;
    panels.reserve(m_navPanelMappings.size());
    for (const auto& mapping : m_navPanelMappings) {
        if (mapping.widget) {
            panels.append(mapping.widget);
        }
    }
    return panels;
}

/** @brief 获取当前面板在映射表中的索引(用于持久化保存) @return 0~N-1索引值，未找到返回-1 */
int NavigationController::currentPanelIndex() const
{
    if (!m_currentPanel) return -1;

    for (int i = 0; i < m_navPanelMappings.size(); ++i) {
        if (m_navPanelMappings[i].widget == m_currentPanel) {
            return i;
        }
    }
    return -1;
}

/** @brief 通过索引恢复面板(启动时使用，不触发动画)，隐藏其他面板并显示目标面板 @param index 面板索引 @return true成功 false越界或widget为空 */
bool NavigationController::restorePanelByIndex(int index)
{
    if (index < 0 || index >= m_navPanelMappings.size()) {
        return false;
    }

    QWidget* target = m_navPanelMappings[index].widget;
    if (!target) return false;

    for (auto* w : allSwitchablePanels()) {
        if (w && w != target) {
            if (w->graphicsEffect()) w->setGraphicsEffect(nullptr);
            w->move(0, 0);
            w->setVisible(false);
        }
    }

    // 确保目标面板位置正确且无残留 effect
    target->setGraphicsEffect(nullptr);
    target->move(0, 0);
    target->setVisible(true);
    m_currentPanel = target;
    syncNavTreeSelection(target);
    emit currentPanelChanged(target);
    ++m_totalRestoresByIndex;  ///< 统计: 通过索引恢复面板次数递增

    return true;
}

/** @brief 设置当前面板(初始化用，不触发动画) @param panel 目标面板widget */
void NavigationController::setCurrentPanel(QWidget* panel)
{
    if (m_currentPanel == panel) return;
    m_currentPanel = panel;
    syncNavTreeSelection(panel);
    emit currentPanelChanged(panel);
}

// lookupPanel/统计计数器/onThemeChanged见 NavigationControllerQuery.cpp
