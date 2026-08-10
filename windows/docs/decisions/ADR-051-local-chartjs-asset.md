# ADR-051：本地 Chart.js 静态资产

## 状态

已接受（2026-08-10）。

## 背景

仪表盘图表原先从 jsDelivr 加载 Chart.js。对于本机、局域网分享和 EXE/离线场景，外部 CDN 不可用
会让核心分析区退化成“图表组件未加载”，且 CSP 必须允许第三方脚本。

## 决策

- 固定当前已验证的 Chart.js `4.4.7` UMD 构建到
  `windows/token_tracker/static/vendor/chart.umd.min.js`，页面通过 Flask static URL 使用，运行时
  不再请求 `cdn.jsdelivr.net`。
- 与该资产一起提交 MIT 许可证文件；升级 Chart.js 必须重新核对 API、许可证、SHA-256、浏览器证据和
  发布审计，不允许直接替换未记录版本。
- `charts.js` 继续只消费全局 `Chart`，业务数据、API 请求、空态和可读文本不迁移到 vendor 资产。
- CSP 的 `script-src` 收敛为 `'self'`；Google Fonts 仍是可选视觉增强，系统字体回退不影响业务运行。

## 官方依据

- [Chart.js 安装文档](https://www.chartjs.org/docs/latest/getting-started/installation.html)：支持下载/静态
  script 集成方式。
- [Chart.js 集成文档](https://www.chartjs.org/docs/latest/getting-started/integration.html)：UMD 构建可
  通过 script tag 暴露 `Chart`。
- [Chart.js MIT License](https://github.com/chartjs/Chart.js/blob/master/LICENSE.md)：随项目保留许可证文本。

## 验证边界

- 已完成：固定资源、许可证、CSP、模板引用和本地静态审计同步。
- 待完成：可用浏览器 CDP 后复核趋势/占比图的真实渲染、空态和 reduced-motion；本地服务需返回
  vendor 资源 `200`。
