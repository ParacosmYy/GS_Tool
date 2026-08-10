/* Author: AI Token Tracker Engineering Team | Maintainer: Project Owner | Purpose: Admin data states and accessible detail rendering. */

import { setLiveMessage } from './modules/live-region.js';

const numberFormatter = new Intl.NumberFormat('zh-CN');
let detailTrigger = null;

/** Fetch a same-origin administrator resource and preserve permission errors. */
async function requestAdmin(path) {
  const response = await fetch(path, { headers: { Accept: 'application/json' } });
  const payload = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(payload?.error?.message || `Request failed (${response.status})`);
  return payload;
}

function setText(id, value) {
  const element = document.getElementById(id);
  if (element) element.textContent = numberFormatter.format(Number(value || 0));
}

function cell(value, tag = 'td') {
  const element = document.createElement(tag);
  element.textContent = value == null ? '' : String(value);
  return element;
}

function emptyRow(body, colSpan, message) {
  const row = document.createElement('tr');
  const empty = cell(message);
  empty.colSpan = colSpan;
  empty.className = 'table-empty';
  body.appendChild(row);
  row.appendChild(empty);
}

function renderUsers(items) {
  const body = document.getElementById('admin-user-rows');
  if (!body) return;
  body.replaceChildren();
  if (!items.length) {
    emptyRow(body, 6, '暂时没有同学账户。');
    return;
  }
  items.forEach((user) => {
    const row = document.createElement('tr');
    row.appendChild(cell(user.username, 'th'));
    row.lastElementChild.scope = 'row';
    const role = cell(user.role);
    role.replaceChildren();
    const roleBadge = document.createElement('span');
    roleBadge.className = 'role-badge';
    roleBadge.textContent = user.role;
    role.appendChild(roleBadge);
    row.appendChild(role);
    row.appendChild(cell(numberFormatter.format(user.calls || 0)));
    row.appendChild(cell(numberFormatter.format(user.total_tokens || 0)));
    row.appendChild(cell(numberFormatter.format(user.work_events || 0)));
    const action = cell('');
    const button = document.createElement('button');
    button.className = 'button button-ghost button-small';
    button.type = 'button';
    button.setAttribute('aria-controls', 'admin-detail');
    button.setAttribute('aria-expanded', 'false');
    button.textContent = '查看';
    button.addEventListener('click', () => {
      detailTrigger = button;
      showUserDetail(user.id, user.username);
    });
    action.appendChild(button);
    row.appendChild(action);
    body.appendChild(row);
  });
}

function renderDetail(activity, name) {
  const title = document.getElementById('admin-detail-title');
  title.replaceChildren();
  title.appendChild(document.createTextNode(`${name} `));
  const subtitle = document.createElement('span');
  subtitle.textContent = 'Activity trace';
  title.appendChild(subtitle);
  const records = document.getElementById('admin-record-rows');
  const eventRows = document.getElementById('admin-event-rows');
  const logRows = document.getElementById('admin-log-rows');
  records.replaceChildren();
  eventRows.replaceChildren();
  logRows.replaceChildren();
  const recordItems = activity.records || [];
  const eventItems = activity.events || [];
  const logItems = activity.logs || [];
  recordItems.forEach((record) => {
    const row = document.createElement('tr');
    [record.timestamp, record.model, numberFormatter.format(record.total_tokens || 0), record.source].forEach((value) => row.appendChild(cell(value)));
    records.appendChild(row);
  });
  eventItems.forEach((event) => {
    const row = document.createElement('tr');
    [event.direction, event.outcome, event.efficiency_score == null ? '—' : `${Number(event.efficiency_score)}%`, event.error_code || event.result_code || '—'].forEach((value) => row.appendChild(cell(value)));
    eventRows.appendChild(row);
  });
  logItems.forEach((log) => {
    const row = document.createElement('tr');
    [log.created_at, log.level, log.event_type, log.error_code || '—', log.message, log.request_id].forEach((value, index) => {
      const logCell = cell(value);
      if (index === 4) logCell.className = 'log-message';
      row.appendChild(logCell);
    });
    logRows.appendChild(row);
  });
  if (!recordItems.length) emptyRow(records, 4, '当前成员暂无 token 记录。');
  if (!eventItems.length) emptyRow(eventRows, 4, '当前成员暂无工作事件。');
  if (!logItems.length) emptyRow(logRows, 6, '当前成员暂无诊断日志。');
  const detail = document.getElementById('admin-detail');
  detail.hidden = false;
  detailTrigger?.setAttribute('aria-expanded', 'true');
  document.getElementById('admin-detail-close')?.focus();
  detail.scrollIntoView({ behavior: 'smooth', block: 'start' });
}

async function showUserDetail(userId, userName) {
  const message = document.getElementById('admin-message');
  setLiveMessage(message, '正在读取成员明细……');
  try {
    const activity = await requestAdmin(`/api/v1/admin/users/${encodeURIComponent(userId)}/records?limit=40`);
    renderDetail(activity, userName);
    setLiveMessage(message, '成员明细已加载。');
  } catch (error) {
    setLiveMessage(message, error.message, true);
  }
}

async function loadAdminData() {
  const status = document.getElementById('admin-status');
  try {
    const [overview, users] = await Promise.all([requestAdmin('/api/v1/admin/overview'), requestAdmin('/api/v1/admin/users')]);
    setText('admin-users', overview.users);
    setText('admin-tokens', overview.usage?.total_tokens);
    setText('admin-events', overview.work_events?.total);
    setText('admin-success', overview.work_events?.success);
    setText('admin-failure', overview.work_events?.failure);
    setText('admin-logs', overview.logs);
    renderUsers(users.items || []);
    status.textContent = 'LIVE / AUDITED';
  } catch (error) {
    setLiveMessage(status, '读取失败', true);
    setLiveMessage(document.getElementById('admin-message'), error.message, true);
  }
}

document.getElementById('admin-detail-close')?.addEventListener('click', () => {
  document.getElementById('admin-detail').hidden = true;
  detailTrigger?.setAttribute('aria-expanded', 'false');
  detailTrigger?.focus();
});
loadAdminData();
