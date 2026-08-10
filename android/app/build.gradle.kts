// Author: AI Token Tracker Engineering Team
// Maintainer: Project Owner
// Purpose: Android client build and safe environment-dependent API configuration.

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
}

val trackerApiBaseUrlRaw = providers.gradleProperty("trackerApiBaseUrl")
    .orElse("http://10.0.2.2:5000/api/v1")
    .get()
val trackerApiBaseUrl = trackerApiBaseUrlRaw
    .replace("\\", "\\\\")
    .replace("\"", "\\\"")

android {
    namespace = "com.aitokentracker"
    compileSdk = 37

    defaultConfig {
        applicationId = "com.aitokentracker"
        minSdk = 23
        targetSdk = 37
        versionCode = 1
        versionName = "0.1.0"

        buildConfigField("String", "API_BASE_URL", "\"$trackerApiBaseUrl\"")
    }

    buildTypes {
        debug {
            manifestPlaceholders["allowCleartext"] = true
        }
        release {
            check(trackerApiBaseUrlRaw.startsWith("https://", ignoreCase = true)) {
                "Release 构建必须通过 -PtrackerApiBaseUrl 配置 HTTPS 服务地址"
            }
            manifestPlaceholders["allowCleartext"] = false
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
            )
        }
    }

    buildFeatures {
        compose = true
        buildConfig = true
    }

    packaging {
        resources.excludes += "/META-INF/{AL2.0,LGPL2.1}"
    }
}

dependencies {
    val composeBom = platform("androidx.compose:compose-bom:2026.06.00")
    implementation(composeBom)
    implementation("androidx.activity:activity-compose:1.13.0")
    implementation("androidx.compose.material3:material3")
    implementation("androidx.compose.ui:ui-tooling-preview")
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.10.0")
    debugImplementation("androidx.compose.ui:ui-tooling")
}
