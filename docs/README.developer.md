# SerialForge

SerialForge 是一个 Windows 原生 Python 嵌入式调试工作站，目标是把串口助手、TCP/UDP 网络调试、BLE GATT 和 J-Link RTT 终端放进同一套可扩展会话模型中。

当前交付切片：**ARCH-125 / UI-1.198** 修复窄窗口下连接配置与 UART 参数横向挤压的问题。窗口不再被 `980px` 硬宽度卡住；连接带按
`REGULAR/COMPACT/NARROW_COMPACT` 三态分行，UART 参数按 `REGULAR/COMPACT/NARROW` 三态排列。端口、波特率、数据位、校验、停止位、流控、
超时和线路控制在窄态会自然纵向展开；控件 identity、bindings、signals、Tab 顺序、业务链路、主题语义与唯一共享 `120Hz` 动效时钟保持不变。
三主题和 `520–1180px` 响应式几何矩阵通过，相关文件均低于 1000 行；独立审查无 Critical/Required，简化审查无必须项；embedded C/C++ public-vendor-source
applicability=N/A。最新 onefile 包已覆盖根目录的 [SerialForge.exe](SerialForge.exe) 与 [SerialForge-latest.exe](SerialForge-latest.exe)，三者均为
`48,057,767` bytes，SHA-256 为 `C8F19055CA977477BA33C087E3CEEA92ACD99F328A2F7F2410189DF96D748BBA`。

当前交付切片：**ARCH-124 / UI-1.197** 将命令页的发送历史与批量命令选择收敛为独立的 `CommandContextBand` 响应式上下文带：宽度足够时两列并排，
窄宽度时自动变为单列；五个批量操作按钮保持独立操作行，不再和选择控件挤在同一条长横排。布局 owner 只重排已有控件，不改变 bindings、signals、
业务执行、焦点/Tab 顺序、滚动 owner、主题语义或唯一共享 120Hz 动效时钟；上下文带不套用嵌套 surface，避免视觉层级堆叠。三主题、320–1180 宽度几何、
workspace 生命周期与 10.2 秒 `120.000Hz` scheduler 样本通过；独立 reviewer `019ff5bc-af7c-7f91-bc2d-891292e96db0` 为
`APPROVE WITH ADVISORIES`（无 Critical/Required），简化审查 `019ff5bc-b25e-7743-9a2b-f0ae71f185f8` 无必须简化项；embedded C/C++ public-vendor-source
applicability=N/A。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与 [SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为
`48,052,563` bytes，SHA-256 为 `B52DD4ADFC7C06CEAF9E2B46BCA1FB9AFB2DE6AA996B286AAF80ED9E12107B16`；打包 revision 为 `local-arch-124`，
archive listing SHA-256 为 `060BCD1C9D88F7C19657FA6A66794FB333F24BE0B8676605BACFBB4A121F9D1D`，provenance verify 通过。签名保持 `NotSigned`，
`release_eligible=false`，`hardware_acceptance=not_run`。

当前交付切片：**ARCH-123 / UI-1.196** 修复命令批处理页的空白失衡与首帧布局瞬态：空态改为自然高度，增加不抢焦点的三步引导轨，
命令页通过显式 `ShortPageVerticalRhythm.TOP` 从顶部开始；三步卡片由同一 `_relayout()` owner 在初始化和 resize 时幂等重排。
不改变批处理信号、ViewModel、执行链路、滚动 owner 或唯一 120Hz 动效时钟。离屏首帧/resize 矩阵覆盖 520/640/720/980/1180 宽度，
三主题与 hide/show/close 通过；10.2 秒 scheduler 为 `1224` frames、`120.000Hz`。独立 reviewer `019ff595-c839-7311-8c47-1a4006951912`
结论为 `APPROVE WITH ADVISORIES`（无 Critical/Required）；简化审查 `019ff595-cb11-7370-b84c-6d088909da57` 无必须简化项。
审查建议的文档同步与首帧幂等修正均已完成；embedded C/C++ public-vendor-source applicability=N/A。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与 [SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为
`48,047,222` bytes，SHA-256 为 `803114A03FA0FBEBF440005A1C79C6970FAB85C0863911CA2CF7B87DDCCB8054`；打包 revision 为
`local-arch-123`，archive listing SHA-256 为 `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify
通过；签名保持 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

历史交付切片：**ARCH-122 / UI-1.195** 修复 Windows 高刷新动效被低精度时间源压到约 64Hz 的问题，并收敛扩展概览指标的响应式密度。
唯一 `MotionController` 继续使用单一 `PreciseTimer`，调度 slot 为 4ms；其内部 elapsed、activity deadline 和动画资格统一使用高精度
`time.perf_counter()`，frame budget 每次最多发出一帧并丢弃过期整数预算，避免事件循环恢复后的 repaint burst。10.2 秒 offscreen scheduler
样本为 `1224` frames、`119.992Hz`、平均 `8.334ms`、p95 `12.097ms`；卡顿注入每次只发 1 帧且 budget 保持在 `[0,1)`。
扩展概览指标按可用宽度在 6/3/2/1 列间重排，长动作文本可收缩换行并保留完整 accessibility description；不改变 DTO、业务、主题、滚动、
OTA/AES/RTT/J-Link 边界。独立 reviewer `019ff57c-bd7a-79b3-838c-6471f5a8d87c` APPROVE、无 Critical/Required findings；简化审查
未发现必须项。三主题、生命周期、真实 Windows GUI/HIDPI、显示器合成、EXE startup 与硬件连接仍需授权验收；119.992Hz 是 scheduler 证据，
不等同显示器实际 120fps。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与 [SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为
`48,044,048` bytes，SHA-256 为 `CF85FAFB7F757FA09853670D051077A31FABECF52A91A8CF205A8145C2DF0B6D`；打包 revision 为 `local-arch-122`，
archive listing SHA-256 为 `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名保持 `NotSigned`，
`release_eligible=false`，`hardware_acceptance=not_run`。

当前交付切片：**ARCH-121 / UI-1.194** 修复总览态工作区的纵向挤压：workspace shell 现在按 root sibling 的实际最小高度预算
计算安全 `minimumHeight`，980×720 下 floor 约为 168px，连接页 viewport 保留可滚动空间；宽屏允许自然扩张但 floor 封顶
220px。focus→overview 恢复后由同一 owner 重同步，resize 首帧通过最多两轮 queued settle 收敛，避免连接配置和实时观测栏
短暂重叠；不改变业务、bindings/signals、原生 QScrollArea、唯一 120Hz MotionController 或 OTA/AES/RTT/J-Link 边界。
三主题×正常/低动效×多尺寸、快速 resize、focus/hide/show/close 共 312 checks、0 failures。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与 [SerialForge-latest.exe](SerialForge-latest.exe)。canonical、
root、root-latest 均为 `48,040,231` bytes，SHA-256 为 `0524910A444C68B5437E94A73481F590E2F4865072E33158C703D1AFB79B80EC`；
打包 revision 为 `local-arch-121`，archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过。签名保持 `NotSigned`，
`release_eligible=false`，`hardware_acceptance=not_run`；真实 Windows GUI/HIDPI、EXE startup 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-120 / UI-1.193** 收敛总览态实时观测栏与发送栏的宽度节奏：动作按钮、显示模式、
CRLF、快捷命令和保存快捷保持 intrinsic width，输入框、发送摘要和状态 rail 吸收剩余空间，980px 下不再
被等权拉宽；不改变 TerminalControlBindings、signals、发送/接收业务、唯一 120Hz MotionController 或主题语义。
三主题×多尺寸总览态与生命周期矩阵 36 项通过。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,038,525`
bytes，SHA-256 为 `05A72BF2599736C65BA4B43CFB525F72FED4EB2C2CA4E47672BAC46B2E07165B`；打包
revision 为 `local-arch-120`，archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。真实 Windows GUI/HIDPI、
EXE startup 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-119 / UI-1.192** 收敛连接页控制带的响应式布局：980px 及以下可用宽度自动切换为
四行紧凑节奏，1180px 及以上恢复常规三行节奏，传输方式、快速配置、连接动作、配置摘要和链路 rail 不再
挤在同一条长横排；控件 identity、signals、业务状态、唯一 120Hz MotionController 和三主题语义保持不变。
三主题×六尺寸×正反向 resize 共 72 项通过，生命周期矩阵 21 项通过。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,036,144`
bytes，SHA-256 为 `CBED1FB08241C4D938796CED3F2AECF84C1A8BBFEC793A9832D489AAFBE495DD`；打包
revision 为 `local-arch-119`，archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE，真实
Windows GUI/HIDPI 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-118 / UI-1.191** 治理快速交互下的动效叠加：主题、工作区、专注模式和传输面板
一次性过渡现在由统一协调器互斥，避免多个过渡同时挤压视觉层；保留唯一 120Hz 共享时钟、原生滚动、
业务状态和 OTA/AES/RTT/J-Link 边界。互斥/早退/生命周期矩阵 75 项通过；调度器 offscreen 样本约 118.33Hz，
不等同真实显示器 120fps。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,033,518`
bytes，SHA-256 为 `83A3EFCB92D13D45C234C5D17D213B8AF22953DA2C8FA267F674EE250683B00F`；打包
revision 为 `local-arch-118`，archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE，真实
Windows GUI/HIDPI 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-117 / UI-1.190** 优化扩展工具站分区节奏：每组标题、说明和能力卡由独立 themed
section 承载，三组能力不再挤成连续长流；保留 980px 两列、1180/1240px OTA 三列、焦点/详情/无障碍、
原生滚动和唯一 120Hz 动效。三主题×三尺寸分区矩阵 318 项、生命周期矩阵 38 项通过。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,031,424`
bytes，SHA-256 为 `4488BDB5E31EEBF6C61D9BE4925C829E64A090E9C158396E6CA3B1A9F98400C0`；打包
revision 为 `local-arch-117`，archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE，真实
Windows GUI/HIDPI 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-116 / UI-1.189** 优化扩展工具站响应式卡片网格：OTA 传输三张能力卡在宽屏
自动排成三列，980px 窄屏保持两列，OTA 安全与 RTT/J-Link 双卡组继续两列；只重排已有卡片，不改变
contract-only/attach-only 语义、选择/焦点/accessibility、原生滚动和唯一 120Hz 动效。三主题×三尺寸、
1→2→3→2→1→3 反向 resize 的卡片几何/身份/滚动矩阵 627 项通过，扩展生命周期矩阵 662 项通过。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,030,866`
bytes，SHA-256 为 `334406DFEA71D5E4B25FB67088DD3B907250EE8529827CE4AB644FB6FEF780DA`；打包
revision 为 `local-arch-116`，archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE，真实
Windows GUI/HIDPI 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-115 / UI-1.188** 优化短页面垂直节奏：共享滚动容器在内容短于 viewport 时
自动垂直居中，长页面继续顶部对齐并保留原生滚动；连接配置不再顶部堆叠、下方留下大块无语义空区，
命令空态既有 Expanding owner、主题、焦点模式和唯一 120Hz MotionController 不变。三主题×980/1180/1240
×四 workspace 的响应式矩阵 217 项、生命周期矩阵 752 项全部通过。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,028,531`
bytes，SHA-256 为 `8E98DB669C51C64813B701015A9DC2E2C9C7BAE4729025278BEECAF218713924`；打包
revision 为 `local-arch-115`，archive listing SHA-256 为
`CD3C60D336971C88071C3A0D1C3B74D6AD090D1D2704F753C13B058E236913A0`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE，真实
Windows GUI/HIDPI 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-114 / UI-1.187** 修复主题工作区布局伸缩权归属：focus 模式把根布局的可用高度
明确交给 themed workspace shell，terminal slot 仅作为可恢复的隐藏 slack；overview 按快照精确恢复根
stretch、子控件可见性、鼠标透明与 size policy。路由条保持固定高度，页面继续使用原生滚动，不再出现
组件挤压或整行白色占位。三主题×980/1180/1240×四 workspace 共 36 个真实 Qt offscreen cases 通过，
生命周期矩阵 118 项检查通过，共享 MotionController 仍以 120Hz 为目标。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,026,927`
bytes，SHA-256 为 `966C16A74D04454CE8780F424E19D99773E3E39B1CBE42A0A5490812CB605F2C`；打包
revision 为 `local-arch-114`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE，真实
Windows GUI/HIDPI 与显示器 120fps 仍需授权环境验收。

当前交付切片：**ARCH-113 / UI-1.186** 修复专注设置页的自然高度与大块空白：focus 模式复用已有透明
terminal slot 作为 layout slack，按当前页面内容结算 shell 高度；短页不再被下方组件挤压，长页保留
原生滚动，focus/overview、route 切换、resize、before-show 和恢复路径不新增时钟。三主题×980/1180/1240
×四 workspace 共 36 个真实 Qt offscreen cases 通过，共享 MotionController 仍以 120Hz 为目标。

当前交付切片：**ARCH-112 / UI-1.185** 收敛紧凑顶栏：以实际 header 宽度为依据，低于 `1120px` 时隐藏
重复的“连接状态”视觉标题，保留状态灯、当前状态文本、状态组 accessibility 语义及主题/动效控制，
宽屏自动恢复。三主题×`980/1040/1120/1240` 实际 Qt 回归通过，紧凑 status cluster 约由 `579px`
收敛到 `536px`，宽屏恢复约 `598px`；共享 `MotionController` 的 120Hz scheduler 未改动。

当前交付切片：**ARCH-111a / UI-1.184** 修正共享 120Hz 动效时钟的长期 frame budget：仍由唯一
8ms `PreciseTimer` 驱动，但按实测 elapsed 累积目标预算，每次 tick 最多一帧，不会在事件循环
恢复后突发补帧；暂停、隐藏、恢复和关闭生命周期保持。真实 Qt offscreen 样本约 `119.61Hz`，
仅代表 scheduler 证据，不等同真实显示器 120Hz。

当前交付切片：**ARCH-110 / UI-1.183** 优化命令管理空态：没有批量命令时，glyph、说明和新建 CTA
在空态画布中水平居中，不再贴左并留下无语义空白；保留唯一 vertical stretch owner、命令执行
语义、无障碍和共享 120Hz 动效时钟。三主题、980×720 / 1240×820 真实 Qt 回归通过。

当前交付切片：**ARCH-109 / UI-1.182** 修复专注设置返回总览时下方组件首帧完全透明造成的空白画布：
布局仍先结算最终位置，实时观测、终端和发送 surface 从 `0.82` 可读透明度淡入到 `1.0`；不新增
timer、布局动画、业务状态或独立动效时钟。980×720、1240×820、三主题、低动效/暂停/隐藏/恢复/
关闭与 120Hz scheduler 回归通过。当前共享 MotionController 仍是 `TARGET_HZ=120`、8ms target，
不等同真实显示器 120fps。

当前路线：**PySide6 + pyserial + PyInstaller**。BLE 作为可选 extra，网络使用 Python 标准库，RTT 先走外部工具桥接。不需要 WSL、Rust、MinGW 或 Visual Studio C++ 编译环境。

当前交付切片：**ARCH-108 / UI-1.181** 修复扩展工具站键盘焦点不可见：详情展开改变内容高度后，
现有滚动 owner 会同步结算 content、确保当前能力卡完整落入 viewport；三主题、980/1240 两种尺寸
下 7 张卡逐一 Tab 聚焦全部可见，横向滚动为 0。没有新增 timer、滚动 owner、业务状态或 OTA/AES/
RTT/J-Link 后端；共享 MotionController 仍为 120Hz target，不等同真实显示器 120fps。

并继续包含 **ARCH-107 / UI-1.180** 优化扩展工具站首屏层级：不再自动重复展示第一张 XMODEM 详情，
只有用户 Tab 聚焦或点击能力卡后才显示对应只读详情；OTA 三协议、AES 安全槽位、RTT/J-Link
attach-only/contract-only 边界保持。真实 Qt offscreen 三主题、980/1240 两种尺寸下，7 张能力卡无重叠，
滚动提示和键盘可访问性保持。共享 MotionController 仍为 120Hz target，不等同真实显示器 120fps。

并继续包含 **ARCH-106 / UI-1.179** 让工作区路线条直接说明当前布局模式：总览显示“总览 · 链路配置”，
专注配置显示“专注 · 解析与遥测/命令管理”等当前页；文案、tooltip 和 accessibility description
复用已有 `WorkspaceContextLabel` 与 `workspaceShell[mode]`，不新增状态源、timer、scroll owner 或
业务耦合。真实 Qt offscreen 三主题、980×720、四 workspace 的 focus/overview 矩阵通过，最长文案
85px、上下文 geometry 158×22，未与滚动提示和路线节点重叠；窄窗口自动 focus 策略保持，以继续保证
配置页可读性。共享 `MotionController` 仍为 120Hz target，不等同真实显示器 120fps。

并继续包含 **ARCH-105 / UI-1.178** 将工作区滚动提示升级为“位置 · 动作”语义：顶部显示
`顶部 · ↓ 向下查看`，中段显示 `中段 · ↕ 上下滚动`，底部显示 `底部 · ↑ 返回顶部`，无滚动时
显示 `全显 · 内容已全部显示`；accessible name 改为“工作区滚动位置”，只复用 native vertical
scrollbar，不新增状态源、timer 或 scroll owner。真实 Qt offscreen 三主题×980/1240、四 workspace
状态矩阵通过，横向 scroll 均为 0，最长提示文案测量为 89px，提示 geometry 为 112×22；共享
MotionController 生命周期在正常、暂停、禁用、隐藏、恢复和关闭路径均通过。120Hz target 仍是
共享时钟目标，不等同真实显示器 120fps。

并继续包含 **ARCH-104 / UI-1.177** 将命令管理空态提升为可伸缩工作区画布：没有批量命令时，
空态是页面唯一的 vertical stretch owner，glyph、说明和 CTA 在画布中垂直居中，不再挤在左上角或
留下无语义空白；批量命令状态、执行逻辑、滚动 owner 和共享动效时钟不变。真实 Qt offscreen
三主题×980/1240、四 workspace 组合无 sibling overlap；专注模式 `1240×820` 的 empty state
为 `(14,151,1176,399)`，滚动提示为 `内容已全部显示`。本轮尝试过 1ms scheduler + elapsed budget，
在真实 `app.exec()` 中未改善 cadence，且可能丢帧，因此已撤回；交付代码继续使用唯一
`MotionController`、`TARGET_HZ=120`、8ms `PreciseTimer` target。最终主循环采样 `frames=116`、均值
`8.536ms`、有效约 `117.16Hz`，只作为 scheduler 证据，不等同真实显示器 120fps。

并继续包含 **ARCH-103 / UI-1.176** 复用已有 `workspaceShell[mode]` 为路线条增加专注/总览
主题层级：专注设置显示 info→history 渐变和蓝色上沿，总览使用中性上沿；不改变路线条高度、Tab
顺序、滚动提示、焦点路径或 transport/session 状态。三主题下 focus/overview 实跑通过，980/1240
四 workspace 横向滚动为 0；共享 MotionController 0.5 秒采样均值 8.41ms（约 118.8Hz），不等同
真实显示器 120fps。并继续包含 **ARCH-102 / UI-1.175** 修复紧凑窗口顶栏密度，并把专注设置↔总览切换改为“先结算布局、
再做透明度过渡”，彻底避免 `QTabWidget` 视口逐帧压缩造成的组件挤压；980×720 连接页 focus viewport
稳定为 514px，顶栏在紧凑态为约 108–113px，1240px 宽度保留完整装饰。真实中间帧 25/55/95/140ms
均无 sibling 几何重叠，三主题×980/1240 横向滚动均为 0。共享 `MotionController` 仍为 120Hz/8ms
target，1 秒离屏采样 119 frames、均值 8.27ms（约 120.9Hz）；该结果不等同真实显示器 120fps。
并继续包含 **ARCH-101 / UI-1.174** 将 page/theme/transport/dialog/focus 的一次性过渡统一迁移到
各自 finite `MotionDrivenAnimationGroup` owner，共用唯一 `MotionController` 的 120Hz/8ms target；
实际连续活动窗口采样约 8.13–9.89ms，并修复 theme sweep 独立启动导致的双时钟风险。并继续包含
**ARCH-100 / UI-1.173** 修复专注设置↔总览过渡使用 Qt 默认动画 driver 时属性更新约 35Hz
导致的中间帧挤压；专注模式的高度和 reveal opacity track 现在由唯一 `MotionController` 的共享帧信号
驱动，目标为 120Hz/8ms，最新真实组合根活动帧间隔均值约 9.04ms。并继续包含 **ARCH-99 / UI-1.172** 修复 980×720
紧凑窗口总览下连接配置页被实时观测、终端和发送区
挤压的问题；连接页在紧凑高度自动进入专注布局，用户显式点击“返回总览”后保持用户选择，宽屏
继续保留原有总览行为。未新增 timer、thread、业务状态、scroll owner 或设备 I/O。并继续包含
**ARCH-98 / UI-1.171** 修复共享动作按钮最小/最大高度冲突的问题；在不改变
OTA/AES/RTT/J-Link contract-only/attach-only 边界的前提下，动作按钮由共享 presentation leaf
统一提供 36px 内容安全高度，命令空态、组件空态和终端空态不再被局部 30px 固定高度压缩；三主题
四 workspace 横向滚动均为 0。共享 MotionController 继续使用 120Hz target、8ms PreciseTimer，
本轮 1 秒离屏采样为 119Hz。并继续包含 **ARCH-97 / UI-1.170** 修复扩展工具站能力卡被通用按钮 QSS 压缩的问题；在不改变
OTA/AES/RTT/J-Link contract-only/attach-only 边界的前提下，7 张卡在三主题和 980/1240 尺寸下
保持内容安全高度。并继续包含 **ARCH-96 / UI-1.169** 将协议最大帧长、UDP 最大报文、TCP Server 最大客户端数和
批量命令步骤延时改为不可编辑有限选项，端口仍保留自由输入；并继续包含 **ARCH-95 / UI-1.168** 的
UART、TCP/UDP/RTT、BLE 八个超时 `ms/s` 选项，用户直接选择常用值，不再手填小数；运行时秒数、默认值和连接 DTO 保持兼容。并继续
包含 **ARCH-94 / UI-1.167** 对“专注设置 ↔ 总览”切换中间帧组件拥挤问题的修复：
既有高度动画保持 220ms，三块实时/终端/发送 surface 在空间让位期间使用一次性主题化 reveal，
完成、反转、隐藏和关闭都会清理临时 effect。ARCH-93 的首次打开自动预选并填入安全的
`UART · 115200 8N1` profile 继续保留，
不自动连接；用户仍只需选择端口并显式点击连接。手动切换 TCP/UDP/BLE/RTT 时会清掉不匹配的旧
preset/context。7 个内置 profile、4 个 workspace、980px 横向滚动、accessibility 和 session gate
离屏验证通过；ARCH-92 的自然高度连接带、唯一 120Hz scheduler 与静态 fan-out 优化继续生效。
ARCH-91 的 ViewModel worker owner、ARCH-90 的 shared motion owner、ARCH-89 的协议/遥测语义分行以及
既有 UART/TCP/UDP/BLE/RTT 边界保持；本轮 GUI/EXE 启动、真实显示器 FPS 与硬件验收仍待授权。

本轮紧凑顶栏与 focus 过渡仍使用各自 presentation-only owner；focus 只在静态布局结算后复用
`MotionDrivenAnimationGroup` 做 opacity 过渡，不再动画 `maximumHeight`。所有一次性过渡只复用
`QPropertyAnimation` 的 easing/interpolation，不启动 Qt 自己的动画时钟。page/theme/dialog 的其他
一次性 fade 已复用同一共享 driver。ARCH-102 架构师 `019ff394-bb5d-7c60-9ce6-c60b65f1ac83`、布局
复核 Terra `019ff39e-e6da-7ec3-8f96-7666b00d5574` 与独立 reviewer
`019ff39b-8db0-7cc1-833e-009e2d0129cb` 均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS；
ARCH-101 架构师 `019ff382-2d72-7d32-a067-09b13c9a32d5` 给出
“每个功能一个 finite owner、全窗口一个共享时钟”的建议；独立 reviewer
`019ff389-959a-77b0-8f26-1e444ee50ac6` 在等待窗口内超时关闭，未形成外部结论，未伪造 PASS；父代理
完成 correctness/architecture/security/performance/readability 五轴 review 与行为保持简化评估。嵌入式
C/C++ public-source applicability 为 N/A。

本轮 onefile 已覆盖根目录：[SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)。canonical、root、root-latest 均为 `48,024,709`
bytes，SHA-256 为 `ED76F5E158A5B951B4A58B4EE1732277B3093C8A08FDD270EA19D57DB539703A`；打包
revision 为 `local-arch-113`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；
本轮未启动 EXE，签名与硬件验收仍以 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`
为准。完整证据见
[`docs/handoffs/current.md`](docs/handoffs/current.md)、
[`docs/handoffs/2026-08-12-arch-113-focus-natural-height.md`](docs/handoffs/2026-08-12-arch-113-focus-natural-height.md)
与 [`docs/adr/0164-focus-natural-height-slack.md`](docs/adr/0164-focus-natural-height-slack.md)。

## 为什么先选 Python

- Windows 上配置最少，适合先快速交付 UART 调试工具；
- PySide6 提供完整桌面控件；
- pyserial 足够先交付 UART；BLE 需要时再安装 `ble` extra；
- PyInstaller 可以把项目打包成 EXE；
- 后续仍通过 domain/application/infrastructure/presentation 分层，保留替换为 Rust 后端的可能性。

Python 不代表把所有逻辑写进 Qt 槽函数。设备 I/O、解析和记录仍必须在独立模块和后台 worker 中运行，UI 只消费批量事件。

## 当前切片与后续路线

ARCH-80 / UI-1.153 继续收敛最小窗口的顶部视觉密度：`workspace.py` 的 header 自身尺寸入口在
`header.width() < 1120` 时只隐藏纯装饰的 `SignalFieldWidget` 与 `themePaletteSwatch`，并把
控制簇间距从 8px 收到 6px；主题下拉、低动效/暂停动效、连接状态和键盘/无障碍语义全部保留，
宽屏恢复两枚装饰件。三主题、980/1119/1120/1240 边界、四工作区和可见滚动页均通过真实组合根
验证；共享 `MotionController` 仍是 `TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target，
本轮 offscreen 事件均值约 15.607ms，仍不宣称精确 120fps。
`local-arch-80` onefile 已覆盖 canonical、根目录 [SerialForge.exe](SerialForge.exe) 与
[SerialForge-latest.exe](SerialForge-latest.exe)，三者均为 `47,987,629` bytes，SHA-256 为
`CB829EBBF27D9C48E1993040E28E07A57A9092EE3EBDC2A1BF32082A9D30F598`；provenance verify 通过，
签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

最新源码切片为 ARCH-79 / UI-1.152：命令管理空批量命令状态通过 trailing stretch 收敛空态卡高度，1240×820 与 980×720 均为约 108px，不再把整个页面拉成空白；共享动效继续由单一 `MotionController` 驱动，28 个 action/空态 surface 仅在 activity/transition 中运动。`TARGET_HZ=120`、`PreciseTimer`、8ms 是 scheduler target，不等同于显示器精确 120fps；本机 Windows offscreen 观测均值约 15.748ms，phase 仍按 elapsed 推进。既有 callbacks、typed `itemData`、外层滚动、三主题、焦点/accessibility、响应式布局和 OTA/debug 独立边界保持；`local-arch-79` onefile 已覆盖根目录 [SerialForge.exe](SerialForge.exe) 与 [SerialForge-latest.exe](SerialForge-latest.exe)，大小 `47,984,076` bytes，SHA-256 为 `085B1BEF964C3E2861183950E28689DAC798E853CA8A2C38E26A279144C324D9`。

UI-1.117 增加 presentation-only 字体运行时适配：优先使用 Windows CJK 系统字体；在 Qt 字体数据库为空且本机标准字体文件存在时注册本机字体，不复制或打包字体资产。三主题、四工作区和 980×680 响应式离屏审计均可正常显示中文且无横向溢出。

UI-1.118 增强现有 Qt scrollbar 的 signal rail：默认、hover、pressed 三态复用既有主题 token，提升配置页滚动 thumb 的可发现性，不改变滚动策略、焦点或键盘行为。

UI-1.119 记录了两次共享动效 fan-out 性能实验：逐帧可见性过滤，以及 Tab 切换时缓存当前页 surface。虽然缓存减少了低层 dispatch 次数，但在同一 offscreen 合成根的端到端测量中更慢，已经撤回；该结论和数据保存在 [`docs/PERFORMANCE.md`](docs/PERFORMANCE.md)，避免把未经证实的复杂度带入生产 UI。

UI-1.120 为扩展工具站的 contract-only / attach-only 能力卡增加主题化 hover 边界，并让两列卡片在 980/1180 宽度均匀伸缩；卡片保持静态说明、NoFocus 和原有只读边界，页面切换继续复用既有一次性淡入。

UI-1.121 为命令管理页批量命令状态 surface 增加语义状态点和 running 呼吸环；状态仍只来自既有 `CommandBatchSnapshot`，步骤轨道、无障碍文案、NoFocus、鼠标透明和 reduced-motion/static fallback 保持不变。

UI-1.122 为协议/Component/Dataset/Curve/Replay 共享状态标签增加左侧 marker 与移动态低对比度 pulse；marker 只读取既有 `state/source`，文字、底部五节点轨道、无障碍、NoFocus 和无横向滚动保持不变。

UI-1.123 修正 Component 空态的 waiting/blocked 反馈：来源不适用时卡片使用 warning/neutral 边界、眉题明确显示 `COMPONENT / BLOCKED`，CTA 隐藏；来源恢复时自动回到等待态和加载入口。

UI-1.124 将连接快速配置上下文的空态/内置/自定义来源分别投影为 neutral/info/history 主题表面，统一复用 `refresh_dynamic_property()`；自绘面板、提示文案和“不自动连接”契约保持不变。

UI-1.125 为链路连接轨道增加非颜色依赖的终态识别：已连接显示完成勾，连接错误显示叉号；连接中仍由共享 MotionController 驱动脉冲，暂停/低动效/关闭时保持静态。

UI-1.126 修复最小窗口下工作区 tab 的硬最小高度与父布局冲突：980×680 时 settings viewport
会在固定 route strip 之前正常收缩，纵向内容继续由现有滚动页承载，避免 tab 内容被路线条覆盖；
不改变 terminal/send、连接/协议状态或动效生命周期。三套主题、980/1180、四工作区组合根审计均无
白色像素、横向滚动或 tab/route overlap。

UI-1.127 增加可选“专注设置 / 返回总览”模式：只折叠实时观测、终端和发送区，并用一次性高度过渡
释放配置页空间；连接、接收、记录与后台 pipeline 继续运行。低动效、暂停、隐藏、最小化和关闭时
静态收敛，三主题和 980/1180 均无白色回退或横向溢出。

UI-1.128 修复连接页 UART/网络/BLE section 标题在专注设置视图中吸收剩余高度的问题；三主题、
980/1180、UART/TCP Client/TCP Server/UDP/BLE/RTT 六种连接方式均保持约 26px 标题高度，
无标题/面板重叠、横向溢出或白色回退。

UI-1.129 修复 UART 无端口时可编辑 combo 的空白输入区：同一语义 placeholder 同时落到
`QComboBox` 与内部 `QLineEdit`，用户可直接看到“未发现端口 · 点击刷新或输入 COMx”，并继续
保留手动输入/清空兼容。三主题 × 980/1180 的真实组合根验证通过，截图见
`build/ui_review_ui129_uart_empty_port.png`。

UI-1.130 修复 UART 参数摘要 rail 在专注模式稳定后被父布局拉伸的问题：纵向空间由约 109px
收敛到 38px，摘要仍保留波特率、帧格式、流控确认和 accessibility 文案；三主题 × 980/1180
通过无白色回退与无横向溢出验证，截图见 `build/ui_review_ui130_uart_summary.png`。

UI-1.131 修复 TCP Client/RTT 说明 hint 在专注模式中吸收剩余高度的问题；文案继续按可用宽度
换行，TCP Client/RTT 的异常高度分别由约 151/111px 收敛到 size hint，六传输 × 三主题 ×
980/1180 无白色回退、横向溢出或标题/面板重叠，截图见 `build/ui_review_ui131_tcp_hint.png`
与 `build/ui_review_ui131_rtt_hint.png`。

UI-1.132 修复命令批量空态 eyebrow/title/hint 被父布局拉伸的问题：文案高度稳定为 14/19/30px，
卡片仍保留可用空间，glyph、CTA、共享动效和长 hint word-wrap 不变；三主题 × 980/1180
通过无白色回退与无横向溢出验证，截图见 `build/ui_review_ui132_command_empty.png`。

UI-1.133 为“扩展 / 工具站”接入概览增加 96×64 资源无关三节点路线 glyph：只消费既有
`ThemeSpec` 与共享 `MotionController` frame，隐藏/暂停/低动效/关闭时静态回退；通过
`ExtensionPanelWidgets` 明确 layout 与 overview 的组合边界，不改变只读规划层、7 张能力卡、
OTA contract-only、RTT/J-Link attach-only 或任何业务状态。三主题 × 980/1180 无横向溢出，
截图见 `build/ui_review_ui133_extension.png`。

UI-1.134 将 UART 端口、数据位、校验、停止位和流控 selector 收敛到 bounded presentation
width，修复 980px 链路页 105px 横向溢出；端口仍可输入 COMx，其余选项仍为直接下拉选择，
itemData、tooltip/accessibility、连接 gate 和 DTO 不变。三主题 × 980/1180 × 六传输及四个
工作区验证通过，截图见 `build/ui_review_ui134_connection_responsive.png`。

UI-1.135 将扩展工具站接入概览提升为分组摘要：application immutable DTO 现在提供 OTA
传输、OTA 安全、调试输出三组能力计数，presentation 以六列 metrics grid 展示并在窄窗口
自动换行；不新增真实后端、动作、timer 或 OTA/J-Link 依赖，当前激活后端仍为 0。

当前交付切片已经覆盖：端口枚举与硬件元数据、完整串口参数、文本/Hex 收发、原始 JSONL 记录、发送历史、命名快捷命令、暂停预览、有限队列、热插拔身份诊断、TCP Client、TCP Server 多客户端、UDP 单播、BLE GATT Central 手动扫描/过滤与 GATT 操作、协议/组件基础切片，以及 attach-only 的 J-Link RTT Telnet 原始字节桥接。协议入口支持 UART/TCP Client 接收的 Raw、Line、Delimiter、Length prefix、XOR-8、CRC16 Modbus、CRC32/IEEE、有界帧预览和 NMEA 0183 `*HH` 行校验；组件入口支持严格 JSON profile v1、固定字段 codec，以及 schema v2 的受限 JSON Pointer/TLV RX codec、表格/过滤和 CSV 视图；原始终端和记录路径保持不变。RTT 复用普通 stream/session 生命周期，连接已有 J-Link Telnet 服务时支持 channel 0/1，并在连接窗口发送 `RTTCh` 配置串；不启动 SEGGER 进程、不加载 SDK/DLL、不提供 memory/halt/flash。TCP Server 默认只监听回环地址，LAN 监听需要显式确认和 IPv4/CIDR allowlist；每次连接使用独立 `PeerId`，多 client 时必须显式选择发送目标，不会广播或把旧连接的目标转投到重连连接；BLE 不自动重连、不自动替换设备；热插拔只提示“设备消失/稳定身份换了端口”，不会静默自动重连。真实 USB-UART 回环、真实 LAN、BLE 扫描/配对/通知、J-Link/目标板和干净 Windows 仍需接入授权环境后验收。

后续继续增加通用协议 codec、字段绑定、数据表/曲线和回放导出。当前 JSON/TLV codec 只处理 UART/TCP Client 的 RX 派生预览；UDP、TCP Server、BLE、RTT 仍保持 raw-only。完整 IDE 调试、烧录、任意脚本、插件市场和云同步不在当前范围内。

为后续嵌入式调试工具站保留了独立的 `ota/`、`debug/` 和 `presentation/` 目录边界：OTA 暂定
预留 XMODEM、YMODEM、TFTP 三个 contract-only 传输槽位，安全层预留带认证的 AES-GCM/CCM
策略与验证端口，debug 层预留 RTT/J-Link 原始打印端口。它们当前没有接入组合根，不代表已经
可以刷写、解密、签名验证或操作探针；真实实现必须按目标 bootloader/MCU 一手资料单独验收。

当前 M5b 已增加独立 Dataset 配置：typed component value 可按有限的
`scale → offset → clamp → enum` 声明式链计算，最近 256 条 sample 默认保留（最大 1024），
并可预览/导出 CSV。Dataset 仍只消费 UART/TCP Client 的组件 RX，不改动 raw、协议帧或 TX。

当前 M5c 已增加历史 RX 回放：读取 recorder JSONL 的原始 RX，严格校验并通过同一套
`协议 → 组件 → Dataset` pipeline 派生预览；支持速度、暂停、恢复、停止、错误/跳过计数，
不会连接设备、发送 TX 或把历史数据伪装成实时会话。回放读取和队列均有界。

当前 M5d 已增加 Dataset 数值曲线：用户选择一个 series 后，使用自绘 Qt Widgets 绘制最多
512 个有限数值点，X 轴为当前窗口的相对捕获时间；错误、字符串和历史/实时来源状态可见，
不引入 QtCharts 或其他图表运行时依赖。

当前 M5e 已增加有界声明式批量命令：用户可以在当前进程内创建最多 16 个固定顺序宏，
每个宏最多 32 步、单步 512 B、总 payload 16 KiB，并为步骤设置有限延时。执行时复用
UART/TCP/UDP/BLE 的现有 typed send，TCP Server 必须显式选择一个 Peer，支持停止和步骤状态；
它不执行脚本、不循环、不等待 ACK、不自动重试，也不支持 RTT 批量发送。宏的“已完成”表示
本地发送队列已接受，不代表设备应用层已处理。

当前 M5f 已增加通用协议预设目录：Raw、Line、Delimiter AA 55、U8 长度前缀和 U16 LE +
CRC16/Modbus 预设只负责填入配置编辑区，必须显式点击“应用”才会重置解析状态；自定义配置
仍可直接编辑。预设是 domain 层不可变 DTO，不绑定传输、Qt 或第三方协议库；Modbus RTU、
MAVLink、NMEA 的完整 profile/codec 仍未宣称完成。

当前 M5g 增加 NMEA 0183 RX line checksum profile：按 LF/CRLF 分帧，校验 `$` 与 `*` 之间的
8-bit XOR 和两个 ASCII 十六进制字符，格式错误与 checksum 不匹配分别可见，成功派生 payload
移除 `*HH` 后缀；原始终端/JSONL 仍保留完整线缆字节。它不解析 RMC/GGA 字段、不做 TX 编码、
不覆盖 NMEA 标准版本全集，也不等于 NMEA 认证兼容。

当前 M5h 增加 Modbus RTU 已分帧 ADU RX 校验组件：保留地址、功能码、异常标记、数据长度、
数据 Hex 和 CRC 对照字段，支持地址/功能码格式错误、截断和 CRC 错误可见。它只校验一个已经
分帧的 256 B 以内 ADU；不会把普通 UART/TCP read chunk 冒充成遵守 t1.5/t3.5 静默间隔的 RTU
stream framer，也不包含完整主从事务、功能码字段语义或 TX 编码。后续 timing framer 与完整
Modbus profile 必须继续保持独立边界。

当前 M5i 增加 MAVLink v1/v2 已分帧 RX validator/profile：按 magic、header、payload length、
sysid/compid、v2 incompat flags、CRC-16/MCRF4XX 和可选 13 B signature 做有界校验；CRC_EXTRA
必须由 profile 显式提供 message-id 映射。缺少映射时状态是 `UNVERIFIED`，v2 signature 只保留并
标记“未认证”，不会伪装成安全验证。它只允许 UART 已分帧 packet，不做 stream resync、完整
dialect/message field decode、signature authentication 或 TX 编码。示例见
`profiles/mavlink-common-heartbeat.json`，其中 `master` 映射版本只适合作为开发示例，生产前必须
固定官方 message-definition revision。

当前 M5m 增加后端实时 stream boundary：MAVLink 可从 UART/TCP stream 中按长度有界提取并对
噪声/坏长度做保守重同步；结构完整但没有 CRC_EXTRA profile 时保持 `UNVERIFIED`，不会伪装成
已验证。Modbus RTU 只有带 timing quality 的 `GapObservation` 才按 t1.5/t3.5 切 ADU；当前
Windows session 提供的是 host read gap，只能作为诊断启发式，不是物理线缆 timestamp。队列丢弃
会形成 source continuity barrier，统计中保留 incomplete/gap/resync/drop。MainWindow 现在可显式
选择并应用两个新 framing；Modbus timing 从当前 UART 的波特率、数据位、校验和停止位派生，
并展示 t1.5/t3.5 与 host-gap 诊断提示；协议栏同时展示 incomplete、parser drop、gap boundary
和 resync。UI-1 已先完成无外部图片/字体资源的 presentation 视觉基础：连接状态区、连接/协议/历史分页、终端
主路径、统一主题 token、曲线 token、分区标题/空态/只读结果表/键盘路径，以及由共享时钟驱动、可独立暂停/低动效的状态脉冲和顶部信号场；
当前为无 IP 自绘几何二次元主题，不包含角色或外部背景资源。真实 UART timing、Modbus 事务、MAVLink dialect/signing、GUI offscreen、J-Link RTT
和正式发行仍未验收。 

当前 UI-1.6 在 UI-1.5 之上完成原创无 IP“星轨霓虹”二次元主题：紫黑/薄荷/淡紫/珊瑚粉 token、Header/Tab/表单/按钮/终端/表格层级、
状态色、星点轨道信号场、状态环形光晕和可暂停/低动效回退。并继续保留 UI-1.5 的状态反馈与动态边界：错误清除后恢复最近正常状态，空发送/Hex 空白输入有明确反馈，
TCP Server 无目标 client 时禁用发送入口；回放/记录按钮补齐可访问状态描述，派生视图在 raw-only 切换后统一恢复快照，
Component/Dataset 空态、Dataset 阅读位置、曲线窄宽度和关闭期间回调 fence 得到补强。UI-1.4 的既有改进仍包括：UART/BLE discovery 入口显示 busy 状态，端点摘要随编辑实时更新，
TCP Server 提前展示 LAN/allowlist 前置条件；终端、协议帧和 Dataset 预览采用 latest-wins presentation 调度，
协议应用/重置会明确提示派生缓存清理范围，曲线在不适用来源下禁用并显示静态空态，批量命令编辑器校验最终
wire payload。该切片仍保持无外部图片/GIF/字体/QSS/QRC 文件、无新增依赖；主题继续使用源码内嵌 QSS、系统字体回退和自绘几何，真实 GUI、读屏、HIDPI、BLE/硬件和 EXE 启动验收需在
授权环境中进行。当前源码 onefile 已生成并复制为根目录 [`SerialForge.exe`](SerialForge.exe)，但该工程包仍是未签名 engineering build，不能等同正式发行。
- UI-1.7 继续把它收束为嵌入式调试工具站：工作区导航改为“链路 / 连接、协议 / 遥测、命令管理”，主观测/发送区建立“实时观测、发送控制”层级；同时为 `QScrollArea` viewport、横向滚动条和 corner 补齐暗色规则，修复系统 palette 留下的一行白色 fallback。当前根目录 `SerialForge.exe` 已按最新源码重新打包，仍是未签名 engineering build。
- UI-1.66 在 Header 的 `SERIALFORGE` wordmark 前增加无外部资源的星轨/S 几何品牌徽记：三套主题使用语义 token，动效复用共享 MotionController，低动效/暂停/隐藏/关闭时保留静态回退，不增加角色、GIF、字体或第三方依赖。
- UI-1.67 将 Header 组织为“品牌层 + 控制层”双层 shell：在 980px 最小窗口内容宽度和 1180px 常用宽度下避免首屏控件拥挤，同时保持连接状态、动效偏好、主题选择和键盘路径不变。
- UI-1.68 为发送控制带增加主题化 `Ctrl+Enter 发送` keycap 提示：只做可发现性展示，不抢焦点、不重复注册快捷键、不改变发送 gate；默认主题和三套切换主题均覆盖非白色文字/背景/边框。
- UI-1.69 将连接配置页 UART/网络/BLE 普通字段统一为主题化次级标签层级，同时保留 section 标题、hint、状态摘要与所有原生交互；避免不同主题下表单标签亮度失衡。
- UI-1.71 将协议/遥测配置页的 8 个普通字段统一为主题化次级标签层级，同时保留 section 标题、状态 owner、动态说明与所有原生交互；不改变协议配置或 parser 语义。
- UI-1.70 为主题选择器的每个原生下拉项增加 ThemeSpec 色盘 icon，让用户在切换前直接识别星轨霓虹、月影深海和樱雾夜航；不新增主题状态或业务依赖。
- UI-1.72 将批量命令编辑对话框的名称、快捷命令、当前步骤、延时和辅助说明统一为主题化次级标签，同时保留空态、错误、编辑校验、popup 和键盘路径。
- UI-1.73 将实时观测工具栏的“显示”字段统一为主题化次级标签，保留显示模式、暂停/记录/接收状态、快捷键与键盘路径。
- UI-1.74 为批量命令编辑与自定义连接 preset 对话框增加 150ms 一次性主题淡入；低动效/暂停静态回退，hide 时清理 effect/animation，
  不改变对话框校验、按钮、焦点、Tab 或原生 dialog 行为。
  本轮 `local-ui-1.74` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；仍为未签名 engineering build，不能等同正式发行。
- UI-1.75 为自定义连接配置编辑对话框增加主题化 metadata surface，并将“名称/备注”纳入 `role="muted"` 次级字段层级；
  不改变 metadata DTO、校验、按钮、初始焦点、Tab 或上一轮淡入清理。
  本轮 `local-ui-1.75` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；仍为未签名 engineering build，不能等同正式发行。
- UI-1.76 为快速配置的选择/清空动作增加一次 420ms 共享 signal rail 确认反馈；复用唯一 `MotionController`，低动效/暂停静态回退，
  不自动连接、不改变 combo/preset DTO 或无障碍语义。
  本轮 `local-ui-1.76` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；仍为未签名 engineering build，不能等同正式发行。
- UI-1.77 修复连接方式切换后的孤立 section 标题行：UART、网络、BLE 标题与对应 panel 在六种 transport 下严格同步，保留 `role="section"`、
  既有 panel fade、焦点/Tab/accessibility 和三套主题语义；不新增业务状态或 timer。
  本轮 `local-ui-1.77` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,892,808` bytes，
  SHA-256 `CBA24073D49B6CA1C6FDC82C873DEBE19FBCB619D2C2CC4580CD5406D9772A88`；仍为未签名 engineering build，不能等同正式发行。
- UI-1.78 为工作区“链路 / 连接、协议 / 遥测、命令管理”的可见用户切换增加一次 320ms shared activity pulse；不新增计时器、状态源或导航
  语义，首次 hydration、隐藏/最小化/关闭、暂停和 reduced-motion 保持静态。
  本轮 `local-ui-1.78` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,890,905` bytes，
  SHA-256 `BFE9346ED94B84E88A871314ACAACE33E5E52D51FBBF0EF86CEBF0AA3CCD1C26`；仍为未签名 engineering build，不能等同正式发行。
- UI-1.79 为错误通知增加一次 520ms shared activity pulse：错误出现时 fault beacon/header signal/status rail 获得即时确认，清除、低动效、暂停、
  隐藏和初始化保持静态，不改变错误模型或清除语义。本轮 `local-ui-1.79` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，
  canonical/root 字节一致；大小 `47,892,391` bytes，SHA-256 `DF374A4497AA1608478CEF24CE89B61038A7BF5AA0445AB6807C83EAADEE07E5`；仍为未签名
  engineering build，不能等同正式发行。
- UI-1.80 为协议→Component→Dataset→Curve 的真实非空派生快照增加一次 360ms shared activity pulse；空快照、初始化、后台页、隐藏/最小化/关闭、暂停
  和低动效保持静态，不改变 DTO、统计、表格/预览/曲线或无障碍语义。本轮 `local-ui-1.80` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，
  canonical/root 字节一致；大小 `47,890,929` bytes，SHA-256 `E47A06C61064CC934B0812FEDCAD462AD61E3DFB664A3593C37705CEEE76365E`；仍为未签名
  engineering build，不能等同正式发行。
- UI-1.81 为历史回放 PLAYING 保留 520ms activity，并为 EOF/STOPPED/ERROR 增加一次 480ms terminal confirmation；history/error rail 增加主题化静态终态
  marker，PAUSED/EMPTY、隐藏/最小化、暂停和低动效保持静态，不改变回放数据与无障碍语义。本轮 `local-ui-1.81` onefile 打包后将覆盖根目录
  [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,893,847` bytes，SHA-256 `DA71E46BE6B626E644F6E907FC75CCE44C8E9C4E174BCA0FC8E6BA9B4B7893D8`；
  仍为未签名 engineering build，不能等同正式发行。
- UI-1.82 为批量命令 RUNNING 保留 520ms activity，并为 COMPLETED/STOPPED/FAILED 增加一次 480ms terminal confirmation；步骤 rail 增加主题化勾/横线/叉
  marker，IDLE、隐藏/最小化、暂停和低动效保持静态，不改变批量数据、按钮、发送队列或无障碍语义。本轮 `local-ui-1.82` onefile 已打包并覆盖根目录
  [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,894,052` bytes，SHA-256 `BD39D94F914E3AF6DC24E44C04A0B02C0BF8F5553C87911D44CE773CBF6489FB`；
  仍为未签名 engineering build，不能等同正式发行。
- UI-1.83 为 Protocol/Component/Dataset/Curve 的 `error`、`blocked`、`history` 状态增加主题化静态叉/双横栏/回退箭头 marker；active/waiting/draft 继续复用
  shared frame，empty/idle、隐藏/最小化、暂停和低动效保持静态，不改变文字、无障碍语义或业务 projection。本轮 `local-ui-1.83` onefile 已打包并覆盖根目录
  [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,891,907` bytes，SHA-256 `2DD17ED968B0BC7045C5443B08B161DD6123EEE1CE6CDDA4D064E8CDDACCBE1D`；
  仍为未签名 engineering build，不能等同正式发行。
- UI-1.84 为连接、协议/遥测、命令管理三个共享设置页显式关闭横向滚动，content 使用可扩展宽度，纵向仍按需滚动；避免底部出现像白线的横向滚动/亮色槽，
  不改变页面内容、键盘顺序或主题语义。本轮 `local-ui-1.84` onefile 已打包并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小
  `47,892,763` bytes，SHA-256 `D10B2F4972EF9D4115B45B87DB770DC66D5E9E7AE91EFE40B40DE070BB838E17`；仍为未签名 engineering build，不能等同正式发行。
- UI-1.85 将稳定 QSS 中 8 处近白 `#fff…` 文字/选中色改为既有 `TEXT`/`SELECTION_TEXT` 语义 token，三主题选择文字、checkbox、combo、table、tab 渲染保持一致，
  不改变交互或业务状态。本轮 `local-ui-1.85` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小
  `47,895,205` bytes，SHA-256 `48A166688A58393D9A060B56DC7C8CCDB3FB9A7F0666EDD6EE3B4C7983AEF6CE`；仍为未签名 engineering build，不能等同正式发行。
- UI-1.86 将表格、表头、状态栏、双向滚动条、corner 与 Tooltip 的稳定 QSS 颜色统一接入已有 surface/border/history/interaction/selection 语义 token，
  保留表头和 Tooltip 渐变层级，三主题 variant override 与原生控件交互不变；`scripts/check.ps1`、compileall、ruff 与三主题 offscreen surface vector 已通过。
  本轮 `local-ui-1.86` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,892,639` bytes，SHA-256
  `18C089F9C700E230F676F1AB1EEDF5F777ED3584D0352C789800CC8AC0BA1683`，archive listing SHA-256
  `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，
  硬件验收 `not_run`。
- UI-1.87 将 stable controls 的状态面、输入/禁用态、按钮、checkbox、workspace Tab、terminal 与终端/批量空态全部接入已有 semantic token，保留 selector、焦点、选择、
  键盘和 accessibility 契约；三主题真实控件组合 offscreen vector 通过，stable template token audit 降至 `legacy_qss_literals=208`。本轮 `local-ui-1.87` onefile 已重新生成并覆盖根目录
  [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,894,441` bytes，SHA-256
  `632E63F3D038A15571194AD89567D66B40C4D0C4452C4C54AD5804AEAF2E4638`，archive listing SHA-256
  `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.88 将 stable shell/base QSS 的 app root、连接/观测/发送 band、状态 badge、pipeline、协议/组件/回放状态和 section/error surface 全部接入 semantic token；base 与 controls
  稳定 hex literal audit 为 `0`，三主题 shell state offscreen vector 通过，selector、property state、动效与 accessibility 不变。本轮 `local-ui-1.88` onefile 已重新生成并覆盖根目录
  [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,895,484` bytes，SHA-256
  `78DB186E8BFD92E48F09A1E8A9830DA16B45F2995912D8A63682C689944A87D4`，archive listing SHA-256
  `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.89 为共享 `MotionController.frame_changed` 增加 lifecycle 末端门禁：关闭、不可见、最小化、暂停/低动效策略关闭时拒绝排队过期帧，并冻结装饰 surface、清除 RX 活跃态；不改变 SessionViewModel、传输、记录、解析或发送语义。源码静态检查、compileall、ruff 和四阻断态/一正常态 offscreen motion vector 已通过。本轮 `local-ui-1.89` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,896,526` bytes，SHA-256 `8D18CF6A28BA1EF0C3062ABDB097AA3F5A12D764B930FBA1F208F131FF8BE565`，archive listing SHA-256 `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.90 新增独立“扩展 / 工具站”只读页：application DTO 展示 XMODEM/YMODEM/TFTP 的契约预留、AES-256-GCM/AES-128-CCM 的安全策略预留、RTT/J-Link 的 attach-only 原始打印边界；不提供 OTA、解密、签名激活或 probe 控制动作。工作区路线和主题化 Tab glyph 扩为四节点，三主题 offscreen capability vector 通过。本轮 `local-ui-1.90` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,911,727` bytes，SHA-256 `82EC5FC027001D4CD34D0114AB0BDC0FC3B5793B14E3B332AD7D0DA5DCC5A36`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.91 修复扩展工具站 capability card 的语义层级：标题恢复为主题正文层级，`contract_only` 使用 info 语义 badge，`attach_only` 使用 history/purple 语义 badge；不新增颜色 token、不改变 DTO、route、Tab、交互或业务动作。三主题 offscreen hierarchy vector、`scripts/check.ps1`、compileall 与 ruff 通过。本轮 `local-ui-1.91` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,914,013` bytes，SHA-256 `30AB41BD37BCE478565413BEB43A19644F608AA0865C023868A77A64889D55AB`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.92 修复工作区 Tab 一次性淡入动效的生命周期：自然完成、快速切换、低动效/暂停、隐藏/关闭 stop 后都会恢复并解绑 page `QGraphicsOpacityEffect`，不改变 Tab、业务状态或动画时长。静态门禁、compileall、ruff 与 offscreen effect lifecycle vector 通过。本轮 `local-ui-1.92` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,915,018` bytes，SHA-256 `E983B2B6EC35B1877F4E46A5063181E4E0DF99262B344DF38FCE8E261035D7B7`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.93 修正扩展 capability card 的可访问语义：标题不再误用 `role=status`，状态 badge 继续保留 `role=status` 与 `state` 属性；不改变 DTO、QSS、卡片布局或 OTA/debug 只读边界。三主题 semantic vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.93` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,913,940` bytes，SHA-256 `69D6713DFD819D1569792A82A5870D8B2927A52D831674E01B3EC94DF2A699D3`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.94 在 UART 参数区增加只读摘要 rail：随波特率、数据位、校验、停止位、流控选择和快速配置即时更新，显示如 `115200 baud · 8N1 · 无流控`；不新增业务状态、不允许手输波特率、不改变连接行为。三主题 UART summary vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.94` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,916,119` bytes，SHA-256 `E6D317841A8983C81CACED40C9798DB70444016E661418589436BBA844F5328D`，archive listing SHA-256 `D41985C7165A66BD66752652C63615CA83A29BC83F22CEED3360D0E176A7F09F`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.95 在扩展工具站顶部增加只读接入概览：从既有 catalog 派生 `7` 个能力槽位、`0` 个已激活后端和“无”当前动作，并明确目标型号/bootloader/授权/验收前置条件；不探测设备、不启用 OTA/J-Link、不新增状态源或动作。三主题概览 vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.95` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,919,227` bytes，SHA-256 `DBABFB57AD3D63393D57B1C3281C1A1C16CC0E7C572928ABFD940FFE660E5618`，archive listing SHA-256 `A11F34E225D750036A4AEE1DF2D77E2AE1B09B1C79F71719DB10FE6512852CA6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- ARCH-7d / UI-1.96 将扩展工具站摘要派生收回 application：`ExtensionStationSummary` 统一约束能力槽位/激活后端/当前动作/接入前置条件，presentation 只负责渲染 immutable DTO；不新增后端、动作、状态源、timer 或依赖。三主题 summary boundary vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.96` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,921,659` bytes，SHA-256 `23B00CF14295A6F5AC399913ED1C8E46EDD8EB1262A4EC3EB1BE2A88367BD950`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.97 将 UART 数据位、校验、停止位和流控下拉本地化为中文用户文案，波特率保持 25 个常用预设且明确不可手输，并补齐 tooltip/accessibility description；底层 enum/data、快速配置和连接行为不变。真实组合根 UART options vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.97` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,920,572` bytes，SHA-256 `D4014B1E307BABA9926339CA04FEDCE37F6E7FFD8813D9CBCEEE30AE58AC54BE`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.98 继续将连接页做成用户可配置的 bounded surface：网络端点、TCP Server allowlist/LAN/客户端、BLE 扫描/过滤/缓存/配对、RTT 通道与 BLE 写入模式均补齐中文上下文提示；RTT/BLE selector 不可手输，底层 data contract、范围和连接行为不变，超时零值显示为“未设置”。真实组合根 connection options vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.98` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,923,829` bytes，SHA-256 `DECC92C2D15BB110FB0977AEAB57A8BDD9E814B36F7037BBAFF0F0E5FBE2D753`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.99 将协议/遥测页的 preset、帧格式、校验、长度前缀、字节序、曲线序列和回放速度变成更易理解的 bounded 中文 selector，并修复 Dataset 动态刷新后英文 fallback；typed data、解析 gate、snapshot、三主题和既有动态时钟不变。真实组合根 protocol options vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.99` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,925,745` bytes，SHA-256 `5DA3C31E0009294AAC921427A94F01BCA578C6A0AD58E2971C1D8193D319F0C4`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.100 让 PipelineSurfaceLabel 消费既有 `state/source` 动态属性：active/transition/draft 使用共享 frame 脉冲，history/blocked/idle 静态展示，节点进度和主题语义色随状态变化；未新增业务状态源、timer、线程、I/O 或依赖。真实组合根 pipeline state vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.100` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,925,850` bytes，SHA-256 `7038F0C57040B6B518A051E1B779D1F58F01DF0B452A8991C54586F62D33BCA5`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.101 在既有 SignalFieldWidget 内增加资源无关的四点星芒、微型彗尾和轨道光点，复用三主题 semantic token 与共享 MotionController，不新增 timer、业务状态、资源或依赖。真实组合根 SignalField vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.101` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,928,874` bytes，SHA-256 `C8A3B9581503479EAFCE01C7675C2F1BDF0D9250CFF45C13ACF4C45B0AC700E7`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.102 为扩展工具站接入概览和 7 张能力卡补齐三主题 semantic surface，并按 contract-only/attach-only 显示 info/history 色带；修复此前透明/原生默认卡片的主题回退。真实组合根 extension surface vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.102` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,928,081` bytes，SHA-256 `717F20E69CFA0120A1DB833509FEEBCDB37AA66199559A6273B75FEF3E1DD9F1`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.103 为工作区外壳 `workspaceShell` 与路线装饰条 `workspaceRouteStrip` 补齐三主题 presentation surface、history→info 语义渐变和边界圆角；不改变 Tab、route beacon、共享动效或业务状态。真实组合根 workspace surface vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.103` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,928,961` bytes，SHA-256 `4F0300C4764F94F25825C8B5795D348BD04D1AFF8A43E6078B7676FA909B70D1`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.104 为 `QStatusBar::item` 与内部 `QStatusBar QLabel` 补齐三主题透明背景、零边界和 muted 文本规则，消除 Qt 原生底部 chrome 的白线回退；不改变 native status text、`StatusFooterSurface`、状态源或共享动效。真实组合根 statusbar chrome vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.104` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,928,792` bytes，SHA-256 `97E43617E1F0B148F713A6049C5338DD3FBF94AE8CE7ACFE9B0F127BCBB8C087`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.105 为核心传输方式选择器 `transportCombo` 补齐 info→input semantic surface、强调字重和 hover/focus/disabled 状态；覆盖 UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT 六种入口，不改变 itemData、不可编辑、accessibility、连接 gate、状态源或共享动效。真实组合根 transport selector vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.105` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,929,551` bytes，SHA-256 `D4B68CBE1D4BB00C4C8F35C4BDC8ED9E4B79BE5EC782C865EE39B31D37A1A7B1`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.106 为主题选择器 `themePicker` 补齐 history→input semantic surface、强调字重和 hover/focus/disabled 状态；保持三个主题 key/icon、itemData、不可编辑、主题切换 signal、一次性 transition、palette swatch 和业务状态不变。真实组合根 theme picker vector、静态门禁、compileall 与 ruff 通过。本轮 `local-ui-1.106` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,929,814` bytes，SHA-256 `82668145CA2F8AA903E3D2BD2A02D8AC4B3B00AAE2DCAD87C9D6F6B42711C2C5`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.107 为连接状态 rail 增加三枚共享帧彗尾、外环与中心光点；仅复用既有 `_phase`、`_animated`、`_state`、节点位置和 `ThemeSpec` 状态语义色，不新增 timer、状态源、公开 API、资源或传输依赖。三主题六状态连接 rail vector、静态门禁、compileall、ruff 与 provenance 校验通过；`opening/open/closing` 动态，`discovered/closed/error` 静态，停止帧稳定。本轮 `local-ui-1.107` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,929,693` bytes，SHA-256 `6785E9F51A9E907A647E2B84A050EE54F93B1925C91B3581DDEEABFC3292434D`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.108 为 transport、终端显示/发送、发送历史、批量命令和批量编辑器选择器补齐显式不可编辑语义、tooltip 与 accessible description；UART 端口保留唯一可手输 combo，并补齐“不会自动连接”说明。真实组合根 27-selector affordance vector、三主题 980/1180 渲染、关闭生命周期、静态门禁、compileall、ruff 与 provenance 校验通过；itemData、signal、连接 gate 和主题行为不变。本轮 `local-ui-1.108` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,930,586` bytes，SHA-256 `27EBF48272AACF3E5CCC3586ABFC8988272A737E80A6D24D57FD001A8B9E6FA4`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.109 为清除错误/终端预览/发送历史、快捷命令、自定义连接保存删除以及 UART 读写超时补齐副作用边界提示与 accessible description；不改变动作、回调、范围、DTO、状态源或连接/发送/原始记录语义。真实组合根 9-action affordance vector、27 个 selector、三主题 980/1180 渲染、关闭生命周期、静态门禁、compileall、ruff 与 provenance 校验通过；本轮 `local-ui-1.109` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,931,714` bytes，SHA-256 `971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`；Python/PyInstaller `3.12.13 / 6.22.0`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- ARCH-6n 将连接控件 enable/hint/busy 刷新从 `MainWindow._update_connection_controls` 纯转发 facade 收敛为组合根绑定的显式 callback，所有 owner controller 行为与首屏/关闭时序不变；本轮
  `local-arch-6n` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,894,630` bytes，SHA-256
  `A56986CF93FB1B3B100C78B3AA172ECF5542BB14BF4BABB8D07537DB32925DB7`；仍为未签名 engineering build，不能等同正式发行。
- ARCH-6o 将 transport 与 protocol framing 两个无状态转发从 `MainWindow` 移到组合根命名 callback，保留 Qt payload、首屏、panel transition 与协议草稿语义；本轮 `local-arch-6o` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,893,668` bytes，SHA-256
  `4DC716FA8B652158058F777284968B8D5E8CC6C3C12DDEF52CBE0B3A919891F8`；仍为未签名 engineering build，不能等同正式发行。
- ARCH-6p 将 protocol/derived/replay/BLE/terminal/lifecycle callback wiring 按 owner 分域移入 bootstrap，`MainWindow` 收敛为构造 + Qt 生命周期 shell；本轮 `local-arch-6p` onefile 已重新生成并覆盖根目录 [`SerialForge.exe`](SerialForge.exe)，canonical/root 字节一致；大小 `47,891,078` bytes，SHA-256
  `70604D4065895FAF07EE86D675FFF8845E7020C82556D0D18ED41188FCA6CA86`；仍为未签名 engineering build，不能等同正式发行。
- ARCH-6w / UI-1.139 将命令管理页批处理控件收敛为 `CommandBatchControlBindings`，由 `bootstrap.py` 在 workspace command builder 完成后唯一组装；commands、connection、command selection、composition 和 lifecycle 通过 `command_batch_bindings_for()` 消费。只携带 Qt wiring，不携带 batch catalog/snapshot、ViewModel、执行策略、timer、callback 或 transport handle；`terminal.py` 仅保留构建阶段装配字段。真实组合根 binding/layout vector、scripts/check.ps1、compileall、Ruff、source-limit、theme-audit 和 provenance 通过；onefile/root/root-latest 已覆盖，canonical/root/root-latest 均为 `47,974,086` bytes，SHA-256 `3BCACA7C90A39C23FF7E4C4605F021E463174AF2BE8032C75496C6E10A0F6327`，archive listing SHA-256 `60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C`。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-79-error-activity.md`](docs/handoffs/2026-08-10-ui-1-79-error-activity.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-80-derived-activity.md`](docs/handoffs/2026-08-10-ui-1-80-derived-activity.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-81-replay-terminal-activity.md`](docs/handoffs/2026-08-10-ui-1-81-replay-terminal-activity.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-82-batch-terminal-activity.md`](docs/handoffs/2026-08-10-ui-1-82-batch-terminal-activity.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-83-analysis-status-marker.md`](docs/handoffs/2026-08-10-ui-1-83-analysis-status-marker.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-84-scroll-surface.md`](docs/handoffs/2026-08-10-ui-1-84-scroll-surface.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-85-theme-text-tokens.md`](docs/handoffs/2026-08-10-ui-1-85-theme-text-tokens.md)，最新入口见 [`docs/handoffs/current.md`](docs/handoffs/current.md)。
上一轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-78-workspace-activity.md`](docs/handoffs/2026-08-10-ui-1-78-workspace-activity.md)。
上一轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-77-transport-section-visibility.md`](docs/handoffs/2026-08-10-ui-1-77-transport-section-visibility.md)。
上一轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-76-preset-activity.md`](docs/handoffs/2026-08-10-ui-1-76-preset-activity.md)。
上一轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-75-preset-metadata-surface.md`](docs/handoffs/2026-08-10-ui-1-75-preset-metadata-surface.md)。
更早一轮交接归档见 [`docs/handoffs/2026-08-10-ui-1-74-dialog-transition.md`](docs/handoffs/2026-08-10-ui-1-74-dialog-transition.md)。
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1.6-anime-theme-package.md`](docs/handoffs/2026-08-10-ui-1.6-anime-theme-package.md)，
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1.7-embedded-station.md`](docs/handoffs/2026-08-10-ui-1.7-embedded-station.md)，
本轮交接归档见 [`docs/handoffs/2026-08-10-ui-1.5-state-surface.md`](docs/handoffs/2026-08-10-ui-1.5-state-surface.md)，
上一轮归档见 [`docs/handoffs/2026-08-10-ui-1.4-feedback-polish.md`](docs/handoffs/2026-08-10-ui-1.4-feedback-polish.md)，
历史 UI-1.3 归档仍见 [`docs/handoffs/2026-08-09-ui-state-lifecycle.md`](docs/handoffs/2026-08-09-ui-state-lifecycle.md)。

当前 M5j 增加 protocol → component → Dataset 的 generation barrier：每个派生事件携带本阶段和
上游 generation，worker 入队时拒绝低于 fence 的旧事件，ViewModel 消费时再次校验 generation
链。配置重置、profile/dataset 切换后的迟到结果不会污染下游统计、样本或 UI；不改变 transport、
raw recorder 或队列容量。M5k 进一步让 replay ingress 携带原始 `RawRecord.session_id` 作为
parser segment；protocol worker 在 FIFO 出队处显式结束旧 partial frame、再为新 segment 建立
干净状态，保持 replay UUID、下游预览和 transport 边界不变。M5l 将本地表单校验也路由到 ViewModel 的
唯一 `ErrorInfo` 状态，MainWindow 只负责渲染；错误详情以控件 tooltip 保留，清除入口统一。
G0 将 Windows 打包入口收敛为单一 `scripts/package.ps1`：core/BLE 与 onedir/onefile
按版本、变体、模式隔离输出，并为每个模式生成 `PROVENANCE.json`、SHA256、PyInstaller
归档清单、依赖树、NOTICE 和第三方清单。当前仍是未签名 engineering build，完整许可证文本、
真实硬件验收和 J-Link RTT 驱动/工具保持未完成。

## 目录

```text
SerialForge/
├─ src/serialforge/       源码：domain/application/infrastructure/presentation/ota/debug
│  ├─ ota/                 OTA 传输与安全升级 contract-only 边界
│  └─ debug/               RTT/J-Link 原始日志 contract-only 边界
├─ scripts/               check.ps1、package.ps1
├─ packaging/             PyInstaller 配置
├─ profiles/              可直接加载的声明式组件示例
├─ docs/                  架构、约束、工作流、路线和 ADR
├─ pyproject.toml         依赖与命令入口
└─ README.md
```

## 环境与运行

推荐使用 Python 3.12 和 `uv`：

```powershell
uv sync --locked --extra dev
.\scripts\run.ps1
```

也可以手动执行 `uv run --locked --extra dev python -m serialforge`。当前项目不要求 Rust、MinGW、WSL 或 C++ 编译器。

`profiles/modbus-rtu-rx-adu.json` 是一个可直接加载的 Modbus RTU RX 示例；它要求当前选择 UART，
且输入已经由上游按 ADU 边界分好。普通 serial read chunk 不能替代 RTU t1.5/t3.5 timing framer。

`profiles/mavlink-common-heartbeat.json` 是一个可直接加载的 MAVLink common HEARTBEAT RX 示例；
它要求当前选择 UART 和已分帧 packet，且只配置 message id 0 的 CRC_EXTRA=50。该映射来自官方
MAVLink common dialect 参考，示例 revision 是 `master`，生产配置必须替换为固定 revision。

需要 BLE 时再执行 `uv sync --locked --extra ble --extra dev`；UART 主线不安装这组额外依赖。

打包时默认生成不含 Bleak/WinRT 的主线包；需要把 BLE 功能放进 EXE 时使用：

```powershell
.\scripts\package.ps1 -Mode onedir -Ble
```

`-Ble` 会额外收集 Bleak 和 Windows WinRT 后端；没有蓝牙实机时仍可启动和查看界面，但不能把扫描、配对、MTU、写入或通知宣称为实机通过。
输出位于 `dist\release\<version>\<core|ble>\<onedir|onefile>`，core 与 BLE 不共用产物目录；
每个模式目录的 manifest 会记录变体、版本、锁文件 hash、PE 版本、签名状态和禁止内容扫描。

### J-Link RTT（最后能力）

RTT 是外部桥接模式：先安装并启动能提供 RTT Telnet 的 J-Link 工具/IDE 和目标调试会话，
再在 SerialForge 选择“J-Link RTT（外部桥接）”，默认连接 `127.0.0.1:19021`，选择 channel
0/1 后连接即可。SerialForge 不自动启动 SEGGER 进程、不分发驱动/SDK/DLL，也不提供
memory、halt、reset、flash 或任意 J-Link 命令。没有 J-Link 工具或硬件时可以验证界面和
标准库 TCP 回环，但不能宣称真实 RTT 已通过。

应用日志和默认原始记录位于 `%LOCALAPPDATA%\SerialForge\logs`，不会写入源码目录或 onefile 临时解压目录。

## 设计原则

- UI 不直接导入 pyserial、bleak 或 socket；
- infrastructure 实现 domain/application 定义的端口；
- Transport、Protocol、Recorder 和 Widget 分离；
- OTA transport、OTA security、debug output 与 presentation 分目录隔离；UI 只消费 DTO/capability，不接触密钥、设备句柄或 vendor SDK；
- 新增传输只增加适配器和配置，不修改终端和记录器；
- 所有后台工作可取消，所有队列和数据缓存有上限；
- 每轮开发按六角色工作流执行，只有一个源码写入者。

详细约束见 [`AGENTS.md`](AGENTS.md)、[`docs/WORKFLOW.md`](docs/WORKFLOW.md) 和 [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)。
每轮协作的最新交接见 [`docs/handoffs/current.md`](docs/handoffs/current.md)，历史交接归档在 [`docs/handoffs/`](docs/handoffs/)。
