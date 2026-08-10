# UI Module 03 — Observatory

负责仪表盘首屏、总 token signal、轨道、趋势图、模型占比、count-up、reveal 和空数据状态。

主文件：`windows/token_tracker/templates/dashboard.html`、`windows/token_tracker/static/app.js`、`static/modules/charts.js`、`static/modules/motion.js`。

图表只消费 API 返回的事实数据；视觉动效不能改变统计口径。

