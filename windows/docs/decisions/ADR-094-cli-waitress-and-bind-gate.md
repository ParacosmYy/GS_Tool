# ADR-094：CLI 默认 Waitress 与显式共享绑定门禁

## 状态

已接受（2026-08-10）。

## 背景

根目录 `start.bat` 已经使用 Waitress，但 `python -m token_tracker serve` 的默认分支仍会
调用 Flask development server。用户按 README 执行 CLI 时会看到开发服务器警告；更重要的是，
本地命令如果收到 `--host 0.0.0.0`，可能在没有显式 LAN 预览确认的情况下把服务暴露到局域网。

## 决策

- `serve` 默认使用 Waitress；只有显式 `--debug` 才允许 Flask development server。
- 未指定 `--lan-preview` 或 `--production` 时，CLI 只接受 loopback 地址。
- 可信 LAN 继续使用 `--lan-preview`，生产分享继续使用 `--production` + HTTPS edge；两者
  复用原有的 Session Secret、provider allowlist、Cookie 和地址预检。
- 保持 `serve` 的参数和 API 路径不变；这是运行时边界收紧，不是业务 API 变更。

## 取舍

- 好处：普通用户不再看到 Flask development server 警告；默认运行时与根入口、EXE 和
  Gateway 的 Waitress 选择一致；误用非 loopback 参数会在创建应用前返回安全错误。
- 代价：需要调试器的开发者必须显式加 `--debug`；需要局域网分享的用户必须显式加
  `--lan-preview`，不能依赖一个隐式的 host 参数。

## 验证边界

- 通过 CLI help、Python 编译、隔离 loopback Waitress 启动和 `/login` 200 验证。
- `serve --host 0.0.0.0` 未加共享模式应返回非零且不监听；未运行 Flask development server。
- 不影响受保护的 5000/5011 服务；真实生产域名、证书和防火墙仍由 HTTPS 部署门禁负责。
