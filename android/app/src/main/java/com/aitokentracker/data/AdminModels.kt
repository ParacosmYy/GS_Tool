/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Sanitized administrator read models for the Android client.
 * Module: Android data / administrator domain contracts.
 *
 * These projections intentionally contain aggregates and bounded activity only.
 * Password hashes, access material, provider keys, and raw prompts never cross
 * the Windows API boundary into this model set.
 */
package com.aitokentracker.data

/** Team-wide totals returned only to a server-authorized administrator. */
data class AdminOverview(
    val totalUsers: Long,
    val usage: UsageTotals,
    val workEvents: AdminWorkEventTotals,
    val logCount: Long,
    val byModel: List<ModelUsage>,
    val trend: List<DailyUsage>,
)

/** Aggregate success/failure counts for structured work signals. */
data class AdminWorkEventTotals(
    val total: Long,
    val success: Long,
    val failure: Long,
)

/** Non-secret per-member aggregate used by the admin member list. */
data class AdminMember(
    val id: Long,
    val username: String,
    val role: String,
    val createdAt: String,
    val calls: Long,
    val inputTokens: Long,
    val outputTokens: Long,
    val totalTokens: Long,
    val workEvents: Long,
    val logs: Long,
)

/** Page metadata returned by the administrator member list. */
data class AdminMembersPage(
    val items: List<AdminMember>,
    val page: PageInfo,
)

/** Identity projection attached to a selected member activity response. */
data class AdminMemberIdentity(
    val id: Long,
    val username: String,
    val role: String,
    val createdAt: String,
)

/** Bounded, sanitized activity for one member selected by an administrator. */
data class AdminMemberActivity(
    val user: AdminMemberIdentity,
    val records: List<UsageRecord>,
    val events: List<WorkEvent>,
    val logs: List<AppLog>,
)
