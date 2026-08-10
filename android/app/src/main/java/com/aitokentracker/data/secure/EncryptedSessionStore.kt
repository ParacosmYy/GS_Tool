/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Encrypt Android session material with a non-exportable Keystore key.
 * Module: Android data / secure local source.
 *
 * SharedPreferences is used only as an encrypted blob container. The access
 * and refresh token plaintext never enters ordinary preferences, logs, or UI.
 */
package com.aitokentracker.data.secure

import android.content.Context
import android.util.Base64
import com.aitokentracker.data.SessionSnapshot
import com.aitokentracker.data.TokenPair
import com.aitokentracker.data.UserProfile
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec
import java.security.KeyStore
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import org.json.JSONObject

private const val ANDROID_KEYSTORE = "AndroidKeyStore"
private const val KEY_ALIAS = "ai-token-tracker.session.v1"
private const val PREFS_NAME = "ai-token-tracker.secure-session"
private const val SESSION_KEY = "encrypted_session"
private const val GCM_TAG_BITS = 128

/** Main-safe encrypted storage; the repository invokes it away from the UI. */
internal class EncryptedSessionStore(context: Context) {
    private val preferences = context.applicationContext.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    @Synchronized
    fun save(pair: TokenPair) {
        val snapshot = JSONObject()
            .put("access_token", pair.accessToken)
            .put("refresh_token", pair.refreshToken)
            .put("expires_at", System.currentTimeMillis() + pair.expiresInSeconds * 1_000L)
            .put("user", JSONObject()
                .put("id", pair.user.id)
                .put("username", pair.user.username)
                .put("role", pair.user.role))
        preferences.edit().putString(SESSION_KEY, encrypt(snapshot.toString())).apply()
    }

    @Synchronized
    fun load(): SessionSnapshot? {
        val encrypted = preferences.getString(SESSION_KEY, null) ?: return null
        return try {
            val root = JSONObject(decrypt(encrypted))
            val user = root.getJSONObject("user")
            SessionSnapshot(
                accessToken = root.getString("access_token"),
                refreshToken = root.getString("refresh_token"),
                expiresAtEpochMillis = root.getLong("expires_at"),
                user = UserProfile(
                    id = user.getLong("id"),
                    username = user.getString("username"),
                    role = user.optString("role", "user"),
                ),
            )
        } catch (_: Exception) {
            // A corrupted or invalidated local session must fail closed.
            clear()
            null
        }
    }

    @Synchronized
    fun clear() {
        preferences.edit().remove(SESSION_KEY).apply()
    }

    private fun encrypt(plaintext: String): String {
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.ENCRYPT_MODE, key())
        val iv = cipher.iv
        val encrypted = cipher.doFinal(plaintext.toByteArray(Charsets.UTF_8))
        return "${encode(iv)}.${encode(encrypted)}"
    }

    private fun decrypt(value: String): String {
        val parts = value.split('.', limit = 2)
        require(parts.size == 2)
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.DECRYPT_MODE, key(), GCMParameterSpec(GCM_TAG_BITS, decode(parts[0])))
        return cipher.doFinal(decode(parts[1])).toString(Charsets.UTF_8)
    }

    private fun key(): SecretKey {
        val store = KeyStore.getInstance(ANDROID_KEYSTORE).apply { load(null) }
        val existing = store.getKey(KEY_ALIAS, null) as? SecretKey
        if (existing != null) return existing
        return KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, ANDROID_KEYSTORE).apply {
            init(
                KeyGenParameterSpec.Builder(
                    KEY_ALIAS,
                    KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT,
                )
                    .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                    .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                    .setUserAuthenticationRequired(false)
                    .build(),
            )
        }.generateKey()
    }

    private fun encode(value: ByteArray): String = Base64.encodeToString(value, Base64.NO_WRAP)

    private fun decode(value: String): ByteArray = Base64.decode(value, Base64.NO_WRAP)
}
