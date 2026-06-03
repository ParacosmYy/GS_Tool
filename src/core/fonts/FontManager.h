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
    static FontManager &instance();
    void initialize();
    QFont font(FontRole role = FontRole::Default, int pointSize = 0) const;
    QFont monospaceFont(int pointSize = 0) const;
    QFont codeFont(int pointSize = 0) const;
    void setRoleFamily(FontRole role, const QString &family);
    QString roleFamily(FontRole role) const;
    void setRoleSize(FontRole role, int pointSize);
    int roleSize(FontRole role) const;
    QString loadCustomFont(const QString &filePath);
    QStringList loadCustomFonts(const QStringList &filePaths);
    QStringList loadedFontFamilies() const;
    void setScaleFactor(double factor);
    double scaleFactor() const;
    int scaledSize(int basePointSize) const;
    void setFallbackChain(FontRole role, const QStringList &families);
    QStringList fallbackChain(FontRole role) const;
    quint64 totalFontRequests() const;
    quint64 totalCustomFontsLoaded() const;
    quint64 totalScaleOps() const;
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
