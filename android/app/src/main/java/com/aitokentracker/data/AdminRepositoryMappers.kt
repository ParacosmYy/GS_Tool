/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Convert remote administrator DTOs into UI-safe domain models.
 * Module: Android data / administrator mapping boundary.
 */
package com.aitokentracker.data

import com.aitokentracker.data.remote.RemoteAdminMember
import com.aitokentracker.data.remote.RemoteAdminMemberActivity
import com.aitokentracker.data.remote.RemoteAdminMembers
import com.aitokentracker.data.remote.RemoteAdminOverview

internal fun RemoteAdminOverview.toDomain(): AdminOverview = AdminOverview(
    totalUsers = totalUsers,
    usage = usage,
    workEvents = AdminWorkEventTotals(
        total = workEvents.total,
        success = workEvents.success,
        failure = workEvents.failure,
    ),
    logCount = logCount,
    byModel = byModel,
    trend = trend,
)

internal fun RemoteAdminMembers.toDomain(): AdminMembersPage = AdminMembersPage(
    items = items.map(RemoteAdminMember::toDomain),
    page = page,
)

internal fun RemoteAdminMember.toDomain(): AdminMember = AdminMember(
    id = id,
    username = username,
    role = role,
    createdAt = createdAt,
    calls = calls,
    inputTokens = inputTokens,
    outputTokens = outputTokens,
    totalTokens = totalTokens,
    workEvents = workEvents,
    logs = logs,
)

internal fun RemoteAdminMemberActivity.toDomain(): AdminMemberActivity = AdminMemberActivity(
    user = AdminMemberIdentity(
        id = user.id,
        username = user.username,
        role = user.role,
        createdAt = user.createdAt,
    ),
    records = records,
    events = events,
    logs = logs,
)
