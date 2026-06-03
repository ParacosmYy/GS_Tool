/**
 * @file ScriptRecorder.cpp
 * @brief 脚本录制器实现 — 录制/回放/保存/加载用户操作序列
 *
 * 布局: 工具栏(录制/播放/停止/保存/加载/清除+速度滑块+状态) + 动作列表。
 * 支持发送数据、延时、连接/断开、注释等动作类型的录制与回放。
 * 脚本以JSON格式保存/加载，回放支持1x-10x变速。
 */

#include "core/widgets/ScriptRecorder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include "core/theme/ThemeManager.h"

/** @brief 构造脚本录制器控件，初始化工具栏按钮和动作列表 @param parent 父控件指针 */
ScriptRecorder::ScriptRecorder(QWidget* parent) : QWidget(parent) {
    setObjectName("scriptRecorder");
    auto* ml = new QVBoxLayout(this); ml->setContentsMargins(8,8,8,8); ml->setSpacing(6);
    auto* tb = new QHBoxLayout; tb->setSpacing(4);
    auto mk = [&](const char* n, const QString& t, bool chk=false)->QPushButton*{
        auto* b=new QPushButton(t,this); b->setObjectName(n); b->setCheckable(chk);
        b->setFixedHeight(28); return b;};
    m_recordBtn=mk("scriptRecordBtn",tr("⏺ 录制"),true);
    m_playBtn=mk("scriptPlayBtn",tr("▶ 播放"));
    m_stopBtn=mk("scriptStopBtn",tr("⏹ 停止")); m_stopBtn->setEnabled(false);
    m_saveBtn=mk("scriptSaveBtn",tr("💾 保存"));
    m_loadBtn=mk("scriptLoadBtn",tr("📂 加载"));
    m_clearBtn=mk("scriptClearBtn",tr("🗑 清除"));
    for(auto* b:{m_recordBtn,m_playBtn,m_stopBtn,m_saveBtn,m_loadBtn,m_clearBtn}) tb->addWidget(b);
    tb->addSpacing(8);
    auto* sl=new QLabel(tr("速度:"),this); sl->setObjectName("scriptSpeedLabel"); tb->addWidget(sl);
    m_speedSlider=new QSlider(Qt::Horizontal,this);
    m_speedSlider->setObjectName("scriptSpeedSlider");
    m_speedSlider->setRange(1,10); m_speedSlider->setValue(5);
    m_speedSlider->setFixedWidth(100); m_speedSlider->setToolTip(tr("回放速度倍率(1x-10x)"));
    tb->addWidget(m_speedSlider); tb->addStretch();
    m_statusLabel=new QLabel(tr("就绪"),this);
    m_statusLabel->setObjectName("scriptStatusLabel"); tb->addWidget(m_statusLabel);
    ml->addLayout(tb);
    m_actionList=new QListWidget(this);
    m_actionList->setObjectName("scriptActionList");
    m_actionList->setAlternatingRowColors(true); ml->addWidget(m_actionList,1);
    m_playbackTimer=new QTimer(this); m_playbackTimer->setSingleShot(true);
    connect(m_playbackTimer,&QTimer::timeout,this,&ScriptRecorder::executeNextAction);
    connect(m_recordBtn,&QPushButton::toggled,this,[this](bool on){on?startRecording():stopRecording();});
    connect(m_playBtn,&QPushButton::clicked,this,&ScriptRecorder::startPlayback);
    connect(m_stopBtn,&QPushButton::clicked,this,&ScriptRecorder::stopPlayback);
    connect(m_saveBtn,&QPushButton::clicked,this,[this](){
        QString p=QFileDialog::getSaveFileName(this,tr("保存脚本"),{},tr("脚本(*.json)"));
        if(!p.isEmpty()) saveToFile(p);});
    connect(m_loadBtn,&QPushButton::clicked,this,[this](){
        QString p=QFileDialog::getOpenFileName(this,tr("加载脚本"),{},tr("脚本(*.json)"));
        if(!p.isEmpty()) loadFromFile(p);});
    connect(m_clearBtn,&QPushButton::clicked,this,&ScriptRecorder::clearScript);
}

/** @brief 获取当前脚本动作序列 @return 脚本动作向量 */
QVector<ScriptAction> ScriptRecorder::script() const { return m_actions; }

/** @brief 加载外部脚本动作序列并刷新列表显示 @param a 要加载的脚本动作向量 */
void ScriptRecorder::loadScript(const QVector<ScriptAction>& a) { m_actions=a; refreshList(); }

/** @brief 清除所有已录制的脚本动作 */
void ScriptRecorder::clearScript() { m_actions.clear(); refreshList(); m_statusLabel->setText(tr("已清除")); }

/** @brief 开始录制用户操作，清空旧动作并进入录制状态 */
void ScriptRecorder::startRecording() {
    m_recording=true; m_recordTimer.start(); m_actions.clear(); refreshList();
    m_recordBtn->setChecked(true); m_playBtn->setEnabled(false);
    m_stopBtn->setEnabled(false); m_statusLabel->setText(tr("录制中..."));
    emit recordingChanged(true);
}

/** @brief 停止录制用户操作，切换到就绪状态 */
void ScriptRecorder::stopRecording() {
    if(!m_recording) return; m_recording=false; m_recordBtn->setChecked(false);
    m_playBtn->setEnabled(true); m_stopBtn->setEnabled(false);
    m_statusLabel->setText(tr("录制完成(%1条)").arg(m_actions.size())); emit recordingChanged(false);
}

/** @brief 录制一条发送数据动作，自动插入延时动作(>50ms时) @param data 发送的数据内容 @param isHex 是否为十六进制格式 */
void ScriptRecorder::recordSendAction(const QString& data, bool isHex) {
    if(!m_recording) return;
    ++m_totalRecords; ++m_totalSends;
    qint64 el=m_recordTimer.elapsed();
    if(!m_actions.isEmpty()&&el>50) m_actions.append({ScriptActionType::Delay,QString::number(el),false,0});
    m_actions.append({ScriptActionType::SendData,data,isHex,el});
    m_recordTimer.restart(); refreshList();
    m_statusLabel->setText(tr("录制中...(%1条)").arg(m_actions.size()));
}

/** @brief 开始回放已录制的脚本动作序列 */
void ScriptRecorder::startPlayback() {
    if(m_actions.isEmpty()){m_statusLabel->setText(tr("无动作可回放"));return;}
    ++m_totalPlaybacks;
    m_playing=true; m_playbackIndex=0; m_recordBtn->setEnabled(false);
    m_playBtn->setEnabled(false); m_stopBtn->setEnabled(true);
    m_statusLabel->setText(tr("回放中...")); emit playbackChanged(true); executeNextAction();
}

/** @brief 停止当前回放并重置到就绪状态 */
void ScriptRecorder::stopPlayback() {
    if(!m_playing) return; m_playing=false; m_playbackTimer->stop(); m_playbackIndex=0;
    m_recordBtn->setEnabled(true); m_playBtn->setEnabled(true);
    m_stopBtn->setEnabled(false); m_statusLabel->setText(tr("回放停止")); emit playbackChanged(false);
}

/** @brief 执行下一个回放动作，发送数据或等待延时后调度下一个动作 */
void ScriptRecorder::executeNextAction() {
    if(!m_playing||m_playbackIndex>=m_actions.size()){if(m_playing)stopPlayback();return;}
    m_actionList->setCurrentRow(m_playbackIndex);
    emit playbackProgress(m_playbackIndex+1,m_actions.size());
    const auto& a=m_actions[m_playbackIndex]; int spd=m_speedSlider->value();
    if(a.type==ScriptActionType::SendData){
        ++m_totalSends;
        emit playbackSendRequested(a.data,a.isHex); m_playbackIndex++; m_playbackTimer->start(10);
    }else if(a.type==ScriptActionType::Delay){
        m_playbackIndex++; m_playbackTimer->start(qMax(1,a.data.toInt()/spd));
    }else{ m_playbackIndex++; m_playbackTimer->start(10); }
}

/** @brief 根据当前动作列表刷新QListWidget显示内容 */
void ScriptRecorder::refreshList() {
    m_actionList->clear();
    for(const auto& a:m_actions){
        QString t=actionIcon(a.type);
        if(a.type==ScriptActionType::SendData) t+=tr("发送: %1%2").arg(a.isHex?"[HEX] ":"",a.data);
        else if(a.type==ScriptActionType::Delay) t+=tr("延时: %1ms").arg(a.data);
        else if(a.type==ScriptActionType::Connect) t+=tr("连接");
        else if(a.type==ScriptActionType::Disconnect) t+=tr("断开");
        else if(a.type==ScriptActionType::Comment) t+=tr("注释: %1").arg(a.data);
        m_actionList->addItem(t);
    }
}

/** @brief 根据动作类型返回对应的图标字符串 @param t 脚本动作类型 @return 图标字符串(emoji) */
QString ScriptRecorder::actionIcon(ScriptActionType t) const {
    switch(t){
    case ScriptActionType::SendData:return"📤 ";case ScriptActionType::Delay:return"⏱ ";
    case ScriptActionType::Connect:return"🔌 ";case ScriptActionType::Disconnect:return"⛔ ";
    case ScriptActionType::Comment:return"📝 ";} return{};
}

/** @brief 将脚本动作序列保存为JSON文件 @param path 保存文件路径 */
void ScriptRecorder::saveToFile(const QString& path) {
    QJsonArray arr;
    for(const auto& a:m_actions) arr.append(QJsonObject{
        {"type",static_cast<int>(a.type)},{"data",a.data},{"isHex",a.isHex},{"timestamp",a.timestamp}});
    QFile f(path);
    if(f.open(QIODevice::WriteOnly)){
        f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
        m_statusLabel->setText(tr("已保存: %1").arg(path));
    }
}

/** @brief 从JSON文件加载脚本动作序列 @param path 加载文件路径 */
void ScriptRecorder::loadFromFile(const QString& path) {
    QFile f(path); if(!f.open(QIODevice::ReadOnly)) return;
    m_actions.clear();
    for(const auto& v:QJsonDocument::fromJson(f.readAll()).array()){
        QJsonObject o=v.toObject();
        m_actions.append({static_cast<ScriptActionType>(o["type"].toInt()),
            o["data"].toString(),o["isHex"].toBool(),o["timestamp"].toInteger()});
    }
    refreshList(); m_statusLabel->setText(tr("已加载(%1条)").arg(m_actions.size()));
}

/** @brief 重置脚本统计计数器(录制/回放/发送) */
void ScriptRecorder::resetScriptStatistics()
{
    m_totalRecords = 0;
    m_totalPlaybacks = 0;
    m_totalSends = 0;
}
