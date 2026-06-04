/**
 * @file FontManager.h
 * @brief 字体管理器 - 统一管理应用内字体加载、缓存和DPI适配
 * @since score-131
 */
#ifndef FONTMANAGER_H
#define FONTMANAGER_H
#include <QFont>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

enum class FontRole { Default, Monospace, Title, Subtitle, Caption, Code, DataDisplay, Toolbar, Status, UserInput, Count_ };

class FontManager : public QObject {
    Q_OBJECT
public:
    /** @brief 获取字体管理器单例引用 */
    static FontManager &instance();
    /** @brief 初始化字体管理器，加载默认字体角色配置 */
    void initialize();
    /** @brief 获取指定角色的字体对象
     *  @param role 字体角色（默认Default）
     *  @param pointSize 字号（0表示使用角色默认值）
     *  @return 配置好的QFont对象 */
    QFont font(FontRole role = FontRole::Default, int pointSize = 0) const;
    /** @brief 获取等宽字体（Monospace角色）
     *  @param pointSize 字号（0表示使用角色默认值）
     *  @return 等宽字体对象 */
    QFont monospaceFont(int pointSize = 0) const;
    /** @brief 获取代码字体（Code角色）
     *  @param pointSize 字号（0表示使用角色默认值）
     *  @return 代码字体对象 */
    QFont codeFont(int pointSize = 0) const;
    /** @brief 设置指定角色的字体族名称
     *  @param role 字体角色
     *  @param family 字体族名称（如"Microsoft YaHei"） */
    void setRoleFamily(FontRole role, const QString &family);
    /** @brief 获取指定角色当前配置的字体族名称
     *  @param role 字体角色
     *  @return 字体族名称 */
    QString roleFamily(FontRole role) const;
    /** @brief 设置指定角色的默认字号
     *  @param role 字体角色
     *  @param pointSize 字号（磅值） */
    void setRoleSize(FontRole role, int pointSize);
    /** @brief 获取指定角色的默认字号
     *  @param role 字体角色
     *  @return 字号（磅值） */
    int roleSize(FontRole role) const;
    /** @brief 从文件加载自定义字体并注册到字体数据库
     *  @param filePath 字体文件路径（.ttf/.otf）
     *  @return 加载成功返回字体族名称，失败返回空串 */
    QString loadCustomFont(const QString &filePath);
    /** @brief 批量加载自定义字体文件
     *  @param filePaths 字体文件路径列表
     *  @return 成功加载的字体族名称列表 */
    QStringList loadCustomFonts(const QStringList &filePaths);
    /** @brief 获取所有已加载的自定义字体族名称列表
     *  @return 字体族名称列表 */
    QStringList loadedFontFamilies() const;
    /** @brief 设置全局字体缩放因子（用于DPI适配）
     *  @param factor 缩放因子（1.0为原始大小） */
    void setScaleFactor(double factor);
    /** @brief 获取当前全局字体缩放因子
     *  @return 缩放因子 */
    double scaleFactor() const;
    /** @brief 根据缩放因子计算实际字号
     *  @param basePointSize 基础字号（磅值）
     *  @return 缩放后的实际字号 */
    int scaledSize(int basePointSize) const;
    /** @brief 设置指定角色的字体回退链（主字体不可用时依次尝试）
     *  @param role 字体角色
     *  @param families 回退字体族名称列表 */
    void setFallbackChain(FontRole role, const QStringList &families);
    /** @brief 获取指定角色的字体回退链
     *  @param role 字体角色
     *  @return 回退字体族名称列表 */
    QStringList fallbackChain(FontRole role) const;
    /** @brief 获取累计字体请求总次数 */
    quint64 totalFontRequests() const;
    /** @brief 获取累计自定义字体加载总次数 */
    quint64 totalCustomFontsLoaded() const;
    /** @brief 获取累计字号缩放计算总次数 */
    quint64 totalScaleOps() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();
signals:
    void fontRoleChanged(FontRole role);
    void customFontLoaded(const QString &family);
    void scaleFactorChanged(double newFactor);
private:
    explicit FontManager(QObject *parent = nullptr);
    ~FontManager() override;
    FontManager(const FontManager &) = delete;
    FontManager &operator=(const FontManager &) = delete;
    struct RoleConfig { QString family; int pointSize = 10; QStringList fallbackFamilies; };
    QMap<FontRole, RoleConfig> m_roles;
    QStringList m_loadedFamilies;
    double m_scaleFactor = 1.0;
    mutable quint64 m_totalFontRequests = 0;
    quint64 m_totalCustomFontsLoaded = 0;
    mutable quint64 m_totalScaleOps = 0;
    QString resolveFamily(FontRole role) const;
};
#endif // FONTMANAGER_H
