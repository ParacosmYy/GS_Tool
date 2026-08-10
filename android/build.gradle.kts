// Author: AI Token Tracker Engineering Team
// Maintainer: Project Owner
// Purpose: Pin the Android build toolchain at the project boundary.

plugins {
    id("com.android.application") version "9.3.0" apply false
    id("org.jetbrains.kotlin.android") version "2.3.21" apply false
    id("org.jetbrains.kotlin.plugin.compose") version "2.3.21" apply false
}
