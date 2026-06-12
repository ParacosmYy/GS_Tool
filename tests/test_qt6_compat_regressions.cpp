#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QModelIndex>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QTcpSocket>
#include <QtQml/QJSEngine>
#include <QtTest/QtTest>

#include "protocol/svd/SvdParser.h"
#include "protocol/svd/SvdRegisterTreeModel.h"
#include "utils/l1/L1Regularization.h"
#include "utils/scripting/ScriptEngine.h"

#define private public
#include "connection/mqtt_client/MqttClientEngine.h"
#undef private

class Qt6CompatRegressionsTest : public QObject
{
    Q_OBJECT

private slots:
    void scriptActionDefaultsToEnabledWhenFieldMissing();
    void scriptActionRespectsExplicitDisabledValue();
    void scriptActionRoundTripsParamsAndTriggers();
    void scriptEngineExecutesSimpleJavaScript();
    void scriptEngineRejectsDisabledAction();
    void l1RegularizationConstructsWithParent();
    void l1SoftThresholdHandlesPositiveNegativeAndDeadZone();
    void l1PredictionUsesDotProduct();
    void l1LassoReturnsOneCoefficientPerFeature();
    void l1StatsCanBeResetAfterFit();
    void mqttRemainingLengthRoundTrips_data();
    void mqttRemainingLengthRoundTrips();
    void mqttRemainingLengthReportsIncompletePayload();
    void mqttTopicFilterAddsBigEndianLengthPrefix();
    void mqttPacketIdsWrapToOneAfterMaxValue();
    void svdParserParsesQt6StringViewElementNames();
    void svdParserReportsMalformedXml();
    void svdTreeModelKeepsDeviceAddressColumnEmpty();
    void svdTreeModelDisplaysPeripheralAndRegisterAddressFields();
    void svdTreeModelReturnsDescriptionOnlyForDisplayRole();
    void svdTreeModelProvidesTooltipForFullDescription();
    void svdTreeModelExposesNodeLevelThroughUserRole();
    void svdTreeModelClearsRowsWhenDeviceIsEmpty();

private:
    static QString minimalSvdXml();
    static SvdDevice parsedDevice();
    static SvdDevice manualDevice();
};

QString Qt6CompatRegressionsTest::minimalSvdXml()
{
    return QStringLiteral(R"XML(
<device>
  <vendor>Acme</vendor>
  <name>ACM32F0</name>
  <description>Regression test MCU</description>
  <addressUnitBits>8</addressUnitBits>
  <width>32</width>
  <peripherals>
    <peripheral>
      <name>GPIOA</name>
      <description>General purpose IO bank A</description>
      <groupName>GPIO</groupName>
      <baseAddress>0x48000000</baseAddress>
      <size>32</size>
      <access>read-write</access>
      <registers>
        <register>
          <name>MODER</name>
          <displayName>Mode register</displayName>
          <description>GPIO port mode register</description>
          <addressOffset>0x00</addressOffset>
          <size>32</size>
          <access>read-write</access>
          <resetValue>0xA8000000</resetValue>
          <fields>
            <field>
              <name>MODER0</name>
              <description>Pin 0 mode</description>
              <bitOffset>0</bitOffset>
              <bitWidth>2</bitWidth>
              <enumeratedValues>
                <enumeratedValue>
                  <name>Input</name>
                  <description>Input mode</description>
                  <value>0</value>
                </enumeratedValue>
                <enumeratedValue>
                  <name>Output</name>
                  <description>Output mode</description>
                  <value>1</value>
                </enumeratedValue>
              </enumeratedValues>
            </field>
          </fields>
        </register>
      </registers>
    </peripheral>
  </peripherals>
</device>
)XML");
}

SvdDevice Qt6CompatRegressionsTest::parsedDevice()
{
    SvdParser parser;
    const bool ok = parser.loadSvdContent(minimalSvdXml());
    Q_ASSERT(ok);
    return parser.device();
}

SvdDevice Qt6CompatRegressionsTest::manualDevice()
{
    SvdField field;
    field.name = QStringLiteral("ENABLE");
    field.description = QStringLiteral("Enable bit");
    field.bitOffset = 0;
    field.bitWidth = 1;
    field.access = QStringLiteral("read-write");

    SvdRegister reg;
    reg.name = QStringLiteral("CTRL");
    reg.description = QStringLiteral("Control register with long description for display truncation checks");
    reg.addressOffset = 0x10;
    reg.size = 32;
    reg.access = QStringLiteral("read-write");
    reg.resetValue = 0x00000001;
    reg.fields.append(field);

    SvdPeripheral periph;
    periph.name = QStringLiteral("USART1");
    periph.description = QStringLiteral("Universal synchronous asynchronous receiver transmitter");
    periph.baseAddress = 0x40011000;
    periph.size = 32;
    periph.access = QStringLiteral("read-write");
    periph.registers.append(reg);

    SvdDevice device;
    device.vendor = QStringLiteral("Acme");
    device.name = QStringLiteral("ACM32F0");
    device.description = QStringLiteral("Manual test device");
    device.addressUnitBits = 8;
    device.width = 32;
    device.peripherals.append(periph);
    return device;
}

void Qt6CompatRegressionsTest::scriptActionDefaultsToEnabledWhenFieldMissing()
{
    QMap<QString, QVariant> map;
    map.insert(QStringLiteral("name"), QStringLiteral("Manual"));
    map.insert(QStringLiteral("code"), QStringLiteral("1 + 1"));
    map.insert(QStringLiteral("language"), 0);
    map.insert(QStringLiteral("triggers"), QStringList{QStringLiteral("manual")});

    const ScriptAction action = ScriptAction::fromVariantMap(map);

    QCOMPARE(action.name, QStringLiteral("Manual"));
    QCOMPARE(action.code, QStringLiteral("1 + 1"));
    QCOMPARE(action.language, ScriptLanguage::JavaScript);
    QCOMPARE(action.triggers, QStringList{QStringLiteral("manual")});
    QVERIFY(action.enabled);
}

void Qt6CompatRegressionsTest::scriptActionRespectsExplicitDisabledValue()
{
    QMap<QString, QVariant> map;
    map.insert(QStringLiteral("name"), QStringLiteral("Disabled"));
    map.insert(QStringLiteral("code"), QStringLiteral("serialWrite('AA')"));
    map.insert(QStringLiteral("language"), 0);
    map.insert(QStringLiteral("enabled"), false);

    const ScriptAction action = ScriptAction::fromVariantMap(map);

    QCOMPARE(action.name, QStringLiteral("Disabled"));
    QVERIFY(!action.enabled);
}

void Qt6CompatRegressionsTest::scriptActionRoundTripsParamsAndTriggers()
{
    ScriptAction original;
    original.name = QStringLiteral("RoundTrip");
    original.code = QStringLiteral("return threshold;");
    original.language = ScriptLanguage::JavaScript;
    original.triggers = QStringList{QStringLiteral("manual"), QStringLiteral("onDataReceived")};
    original.params.insert(QStringLiteral("threshold"), QStringLiteral("42"));
    original.enabled = false;

    const ScriptAction restored = ScriptAction::fromVariantMap(original.toVariantMap());

    QCOMPARE(restored.name, original.name);
    QCOMPARE(restored.code, original.code);
    QCOMPARE(restored.language, original.language);
    QCOMPARE(restored.triggers, original.triggers);
    QCOMPARE(restored.params, original.params);
    QCOMPARE(restored.enabled, original.enabled);
}

void Qt6CompatRegressionsTest::scriptEngineExecutesSimpleJavaScript()
{
    ScriptEngine engine;

    const ScriptResult result = engine.executeCode(QStringLiteral("21 * 2"));

    QVERIFY2(result.success, qPrintable(result.error));
    QCOMPARE(result.returnValue.toInt(), 42);
    QCOMPARE(engine.stats().value(QStringLiteral("totalExecutions")).toULongLong(), 1ULL);
}

void Qt6CompatRegressionsTest::scriptEngineRejectsDisabledAction()
{
    ScriptEngine engine;
    ScriptAction action;
    action.name = QStringLiteral("Disabled");
    action.code = QStringLiteral("1 + 1");
    action.enabled = false;

    const ScriptResult result = engine.execute(action);

    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QStringLiteral("禁用")));
}

void Qt6CompatRegressionsTest::l1RegularizationConstructsWithParent()
{
    QObject parent;
    L1Regularization regularizer(&parent);

    QCOMPARE(regularizer.parent(), &parent);
    QCOMPARE(regularizer.stats().totalFitted, 0);
    QCOMPARE(regularizer.stats().totalPredictions, 0);
    QCOMPARE(regularizer.stats().avgProcessingTimeMs, 0.0);
}

void Qt6CompatRegressionsTest::l1SoftThresholdHandlesPositiveNegativeAndDeadZone()
{
    QCOMPARE(L1Regularization::softThreshold(5.0, 2.0), 3.0);
    QCOMPARE(L1Regularization::softThreshold(-5.0, 2.0), -3.0);
    QCOMPARE(L1Regularization::softThreshold(1.0, 2.0), 0.0);
    QCOMPARE(L1Regularization::softThreshold(-1.0, 2.0), 0.0);
}

void Qt6CompatRegressionsTest::l1PredictionUsesDotProduct()
{
    const QVector<double> coefficients{1.5, -2.0, 0.25};
    const QVector<double> sample{2.0, 3.0, 4.0};

    QCOMPARE(L1Regularization::predict(coefficients, sample), -2.0);
}

void Qt6CompatRegressionsTest::l1LassoReturnsOneCoefficientPerFeature()
{
    L1Regularization regularizer;
    const QVector<QVector<double>> x{
        QVector<double>{1.0, 0.0},
        QVector<double>{0.0, 1.0},
        QVector<double>{1.0, 1.0},
        QVector<double>{2.0, 1.0}
    };
    const QVector<double> y{1.0, 2.0, 3.0, 4.0};

    const QVector<double> coefficients = regularizer.lasso(x, y, 0.01, 40, 1e-8);

    QCOMPARE(coefficients.size(), 2);
    QVERIFY(std::isfinite(coefficients.at(0)));
    QVERIFY(std::isfinite(coefficients.at(1)));
    QCOMPARE(regularizer.stats().totalFitted, 1);
}

void Qt6CompatRegressionsTest::l1StatsCanBeResetAfterFit()
{
    L1Regularization regularizer;
    const QVector<QVector<double>> x{
        QVector<double>{1.0},
        QVector<double>{2.0},
        QVector<double>{3.0}
    };
    const QVector<double> y{1.0, 2.0, 3.0};

    regularizer.lasso(x, y, 0.0, 20, 1e-8);
    QVERIFY(regularizer.stats().totalFitted > 0);

    regularizer.resetStatistics();

    QCOMPARE(regularizer.stats().totalFitted, 0);
    QCOMPARE(regularizer.stats().totalPredictions, 0);
    QCOMPARE(regularizer.stats().avgProcessingTimeMs, 0.0);
}

void Qt6CompatRegressionsTest::mqttRemainingLengthRoundTrips_data()
{
    QTest::addColumn<int>("length");
    QTest::addColumn<QByteArray>("encoded");

    QTest::newRow("zero") << 0 << QByteArray::fromHex("00");
    QTest::newRow("single-byte-max") << 127 << QByteArray::fromHex("7f");
    QTest::newRow("two-byte-min") << 128 << QByteArray::fromHex("8001");
    QTest::newRow("two-byte-value") << 321 << QByteArray::fromHex("c102");
    QTest::newRow("three-byte-value") << 16384 << QByteArray::fromHex("808001");
    QTest::newRow("mqtt-max") << 268435455 << QByteArray::fromHex("ffffff7f");
}

void Qt6CompatRegressionsTest::mqttRemainingLengthRoundTrips()
{
    QFETCH(int, length);
    QFETCH(QByteArray, encoded);

    MqttClientEngine engine;
    int offset = 0;

    const QByteArray actual = engine.encodeRemainingLength(length);
    const int decoded = engine.decodeRemainingLength(actual, &offset);

    QCOMPARE(actual, encoded);
    QCOMPARE(decoded, length);
    QCOMPARE(offset, encoded.size());
}

void Qt6CompatRegressionsTest::mqttRemainingLengthReportsIncompletePayload()
{
    MqttClientEngine engine;
    int offset = 0;

    const int decoded = engine.decodeRemainingLength(QByteArray::fromHex("80"), &offset);

    QCOMPARE(decoded, -1);
    QCOMPARE(offset, 0);
}

void Qt6CompatRegressionsTest::mqttTopicFilterAddsBigEndianLengthPrefix()
{
    MqttClientEngine engine;

    const QByteArray filter = engine.buildTopicFilter(QStringLiteral("device/+/rx"));

    QCOMPARE(filter.left(2), QByteArray::fromHex("000b"));
    QCOMPARE(filter.mid(2), QByteArray("device/+/rx"));
}

void Qt6CompatRegressionsTest::mqttPacketIdsWrapToOneAfterMaxValue()
{
    MqttClientEngine engine;
    engine.m_nextPacketId = 65535;

    const quint16 maxId = engine.nextPacketId();
    const quint16 wrappedId = engine.nextPacketId();

    QCOMPARE(maxId, static_cast<quint16>(65535));
    QCOMPARE(wrappedId, static_cast<quint16>(1));
}

void Qt6CompatRegressionsTest::svdParserParsesQt6StringViewElementNames()
{
    SvdParser parser;
    QSignalSpy completedSpy(&parser, &SvdParser::parseCompleted);

    const bool ok = parser.loadSvdContent(minimalSvdXml());

    QVERIFY2(ok, qPrintable(parser.lastError()));
    QCOMPARE(completedSpy.count(), 1);
    QCOMPARE(parser.device().name, QStringLiteral("ACM32F0"));
    QCOMPARE(parser.device().vendor, QStringLiteral("Acme"));
    QCOMPARE(parser.totalPeripherals(), 1ULL);
    QCOMPARE(parser.totalRegisters(), 1ULL);
    QCOMPARE(parser.totalFields(), 1ULL);
    QCOMPARE(parser.peripheral(QStringLiteral("GPIOA")).baseAddress, static_cast<quint32>(0x48000000));
    QCOMPARE(parser.register_(QStringLiteral("GPIOA"), QStringLiteral("MODER")).resetValue,
             static_cast<quint64>(0xA8000000));
}

void Qt6CompatRegressionsTest::svdParserReportsMalformedXml()
{
    SvdParser parser;
    QSignalSpy errorSpy(&parser, &SvdParser::parseError);

    const bool ok = parser.loadSvdContent(QStringLiteral("<device><name>Broken</device>"));

    QVERIFY(!ok);
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(!parser.lastError().isEmpty());
    QCOMPARE(parser.totalParseErrors(), 1ULL);
}

void Qt6CompatRegressionsTest::svdTreeModelKeepsDeviceAddressColumnEmpty()
{
    SvdRegisterTreeModel model;
    model.setDevice(parsedDevice());

    const QModelIndex deviceAddress = model.index(0, SvdRegisterTreeModel::ColAddress);

    QVERIFY(deviceAddress.isValid());
    QCOMPARE(model.data(deviceAddress, Qt::DisplayRole).toString(), QString());
}

void Qt6CompatRegressionsTest::svdTreeModelDisplaysPeripheralAndRegisterAddressFields()
{
    SvdRegisterTreeModel model;
    model.setDevice(manualDevice());

    const QModelIndex deviceIndex = model.index(0, SvdRegisterTreeModel::ColName);
    const QModelIndex peripheralAddress = model.index(0, SvdRegisterTreeModel::ColAddress, deviceIndex);
    const QModelIndex registerOffset = model.index(0, SvdRegisterTreeModel::ColOffset,
                                                   model.index(0, SvdRegisterTreeModel::ColName, deviceIndex));
    const QModelIndex registerReset = model.index(0, SvdRegisterTreeModel::ColResetValue,
                                                  model.index(0, SvdRegisterTreeModel::ColName, deviceIndex));

    QCOMPARE(model.data(peripheralAddress, Qt::DisplayRole).toString(), QStringLiteral("0X40011000"));
    QCOMPARE(model.data(registerOffset, Qt::DisplayRole).toString(), QStringLiteral("0X00000010"));
    QCOMPARE(model.data(registerReset, Qt::DisplayRole).toString(), QStringLiteral("0X00000001"));
}

void Qt6CompatRegressionsTest::svdTreeModelReturnsDescriptionOnlyForDisplayRole()
{
    SvdRegisterTreeModel model;
    model.setDevice(manualDevice());

    const QModelIndex deviceIndex = model.index(0, SvdRegisterTreeModel::ColName);
    const QModelIndex peripheralIndex = model.index(0, SvdRegisterTreeModel::ColName, deviceIndex);
    const QModelIndex registerDescription = model.index(0, SvdRegisterTreeModel::ColDescription, peripheralIndex);

    const QString display = model.data(registerDescription, Qt::DisplayRole).toString();

    QVERIFY(display.startsWith(QStringLiteral("Control register")));
    QVERIFY(display.size() <= 80);
}

void Qt6CompatRegressionsTest::svdTreeModelProvidesTooltipForFullDescription()
{
    SvdRegisterTreeModel model;
    model.setDevice(manualDevice());

    const QModelIndex deviceIndex = model.index(0, SvdRegisterTreeModel::ColName);
    const QModelIndex peripheralIndex = model.index(0, SvdRegisterTreeModel::ColName, deviceIndex);
    const QModelIndex registerName = model.index(0, SvdRegisterTreeModel::ColName, peripheralIndex);
    const QString tooltip = model.data(registerName, Qt::ToolTipRole).toString();

    QVERIFY(tooltip.contains(QStringLiteral("<b>CTRL</b>")));
    QVERIFY(tooltip.contains(QStringLiteral("偏移: 0X00000010")));
    QVERIFY(tooltip.contains(QStringLiteral("复位值: 0X00000001")));
}

void Qt6CompatRegressionsTest::svdTreeModelExposesNodeLevelThroughUserRole()
{
    SvdRegisterTreeModel model;
    model.setDevice(manualDevice());

    const QModelIndex deviceIndex = model.index(0, SvdRegisterTreeModel::ColName);
    const QModelIndex peripheralIndex = model.index(0, SvdRegisterTreeModel::ColName, deviceIndex);
    const QModelIndex registerIndex = model.index(0, SvdRegisterTreeModel::ColName, peripheralIndex);
    const QModelIndex fieldIndex = model.index(0, SvdRegisterTreeModel::ColName, registerIndex);

    QCOMPARE(model.data(deviceIndex, Qt::UserRole).toInt(),
             static_cast<int>(SvdTreeNode::Level::Device));
    QCOMPARE(model.data(peripheralIndex, Qt::UserRole).toInt(),
             static_cast<int>(SvdTreeNode::Level::Peripheral));
    QCOMPARE(model.data(registerIndex, Qt::UserRole).toInt(),
             static_cast<int>(SvdTreeNode::Level::Register));
    QCOMPARE(model.data(fieldIndex, Qt::UserRole).toInt(),
             static_cast<int>(SvdTreeNode::Level::Field));
}

void Qt6CompatRegressionsTest::svdTreeModelClearsRowsWhenDeviceIsEmpty()
{
    SvdRegisterTreeModel model;
    model.setDevice(manualDevice());
    QVERIFY(model.rowCount() > 0);

    model.clear();

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.totalNodeCount(), 0);
}

QTEST_MAIN(Qt6CompatRegressionsTest)
#include "test_qt6_compat_regressions.moc"
