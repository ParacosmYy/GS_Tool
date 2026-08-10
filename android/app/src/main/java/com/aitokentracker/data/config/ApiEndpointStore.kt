/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Validate and persist the non-secret Windows API endpoint.
 * Module: Android data / local configuration.
 *
 * Only the service address is stored here. Credentials and bearer material are
 * deliberately owned by EncryptedSessionStore and never share this preference.
 */
package com.aitokentracker.data.config

import android.content.Context
import java.net.URI

private const val PREFERENCES_NAME = "ai-token-tracker.connection"
private const val ENDPOINT_KEY = "api_base_url"
private const val MAX_ENDPOINT_LENGTH = 300

/** Local-first endpoint configuration for emulator, LAN, and HTTPS deployments. */
internal class ApiEndpointStore(
    context: Context,
    defaultEndpoint: String,
    private val allowInsecureHttp: Boolean,
) {
    private val preferences = context.applicationContext.getSharedPreferences(
        PREFERENCES_NAME,
        Context.MODE_PRIVATE,
    )
    private val safeDefault = normalizeApiBaseUrl(defaultEndpoint, allowInsecureHttp)

    fun load(): String {
        val stored = preferences.getString(ENDPOINT_KEY, null) ?: return safeDefault
        return runCatching { normalizeApiBaseUrl(stored, allowInsecureHttp) }.getOrDefault(safeDefault)
    }

    fun save(endpoint: String): String {
        val normalized = normalizeApiBaseUrl(endpoint, allowInsecureHttp)
        preferences.edit().putString(ENDPOINT_KEY, normalized).apply()
        return normalized
    }
}

/** Validate an endpoint before it can reach the HTTP adapter. */
internal fun normalizeApiBaseUrl(rawEndpoint: String, allowInsecureHttp: Boolean = true): String {
    val value = rawEndpoint.trim().trimEnd('/')
    require(value.length in 1..MAX_ENDPOINT_LENGTH) { "API 地址长度无效" }
    require(value.none { it.isWhitespace() }) { "API 地址不能包含空白字符" }
    val uri = runCatching { URI(value) }.getOrElse { throw IllegalArgumentException("API 地址格式无效") }
    val scheme = uri.scheme?.lowercase()
    require(scheme == "http" || scheme == "https") { "API 地址必须使用 http 或 https" }
    require(allowInsecureHttp || scheme == "https") { "发布版本的 API 地址必须使用 HTTPS" }
    require(!uri.host.isNullOrBlank()) { "API 地址缺少主机名" }
    require(uri.rawUserInfo == null) { "API 地址不能携带用户名或密码" }
    require(uri.rawQuery == null && uri.rawFragment == null) { "API 地址不能包含 query 或 fragment" }
    return value
}
