/**
 * @file IPanelProvider.h
 * @brief 面板提供者接口 — 插件/模块注册自定义面板的统一契约
 *
 * 插件和模块通过此接口向PanelManager注册自定义面板。
 * PanelManager负责实际的创建和布局管理。
 * 层级: L0 纯虚接口层
 */
#ifndef IPANELPROVIDER_H
#define IPANELPROVIDER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class QWidget;

/** @brief 面板区域枚举 — 描述面板在主窗口中的停靠位置 */
enum class PanelArea {
    Left,           ///< 左侧导航树区域
    Center,         ///< 中央内容区域(默认)
    Right,          ///< 右侧属性/详情区域
    Bottom,         ///< 底部状态/日志区域
    Floating,       ///< 浮动窗口
    TabCenter       ///< 中央Tab页(多面板共享)
};

/** @brief 面板元数据 — 描述面板的静态属性 */
struct PanelMeta {
    QString id;             ///< 面板唯一标识(如 "terminal")
    QString name;           ///< 面板显示名称(tr()包裹)
    QString icon;           ///< 图标名称(IconManager)
    PanelArea area;         ///< 默认停靠区域
    int order = 0;          ///< 同区域内排列优先级
    bool singleton = true;  ///< 仅允许单个实例
    bool closable = true;   ///< 用户可关闭
    bool floatingDefault = false; ///< 默认浮动显示
    QStringList tags;       ///< 搜索标签(CommandPalette)
    QVariantMap config;     ///< 自定义配置参数
};

/**
 * @brief 面板提供者接口
 * 协作: PanelManager(创建/布局) / PluginApi(注册) / CommandPalette(搜索)
 */
class IPanelProvider : public QObject {
    Q_OBJECT
public:
    explicit IPanelProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IPanelProvider() = default;
    virtual PanelMeta meta() const = 0;              ///< 获取面板元数据
    virtual QWidget* create(QWidget* parent) = 0;    ///< 创建面板实例
    virtual bool isInitialized() const = 0;          ///< 是否已初始化
    virtual void teardown() = 0;                     ///< 销毁前释放资源
    virtual QVariantMap saveState() const = 0;       ///< 保存运行时状态
    virtual void restoreState(const QVariantMap& s) = 0; ///< 恢复运行时状态
signals:
    void metaChanged(const PanelMeta& meta);         ///< 元数据变化
    void activationRequested();                       ///< 请求激活面板
};

Q_DECLARE_METATYPE(PanelArea)
Q_DECLARE_METATYPE(PanelMeta)
#endif // IPANELPROVIDER_H
