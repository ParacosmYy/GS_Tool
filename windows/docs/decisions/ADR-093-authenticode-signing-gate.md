# ADR-093：Windows EXE Authenticode 签名门禁

**作者：** AI Token Tracker Engineering Team
**维护者：** ARCH-2 / Release Owner
**状态：** Accepted；实际签名 pending
**日期：** 2026-08-10
**前置：** ADR-089、ADR-091、ADR-092

## 背景

当前 EXE 已具备 manifest、SHA-256、升级/回滚和 LocalAppData 数据边界，但 Windows 用户在正式分发时还需要可信的 Authenticode 签名。项目当前环境没有可确认的 `signtool.exe` 和正式发布证书，不能把本地证书或 unsigned 文件描述成生产签名。

## 决策

1. `packaging/sign-build.ps1` 只在构建目录上执行签名，并要求调用者显式提供证书 thumbprint、证书私钥、Code Signing EKU、未过期证书和 HTTPS timestamp URL。
2. 每个 `.exe`、`.dll` 和 `.pyd` PE 文件都使用 SHA-256 文件摘要与 SHA-256 时间戳摘要签名；`signtool` 非零退出码立即 fail-closed。签名完成后必须重新执行 `package.ps1`，让 `RELEASE-MANIFEST.json` 捕获签名后的 EXE 哈希。
3. `packaging/verify-signature.ps1` 只读检查所有 PE 文件，只有 `Get-AuthenticodeSignature` 返回 `Valid` 才允许通过；可选 thumbprint 不匹配时失败。unsigned、过期、未信任或自签名状态不能被映射为 pass。
4. 签名工具、证书、私钥和 timestamp 服务属于发布负责人管理的外部资源，不进入 Git、ZIP、日志或项目 `.env`。

## 使用顺序

```powershell
.\packaging\build.ps1
.\packaging\sign-build.ps1 `
  -CertificateThumbprint "<approved-code-signing-thumbprint>" `
  -TimestampUrl "https://<approved-timestamp-service>"
.\packaging\package.ps1 -Version 0.1.0
.\packaging\verify-signature.ps1 `
  -PackageDirectory .\dist\AI-Token-Tracker `
  -ExpectedThumbprint "<approved-code-signing-thumbprint>"
```

## 当前证据

- 源码质量审计已检查两个签名入口和本 ADR。
- 当前机器的 `signtool.exe` 不可用；没有获得发布负责人批准的证书/时间戳服务材料，因此没有执行或伪造签名通过证据。
- R-10 继续保持 `conditional`，剩余门禁为批准的签名环境和正式分发渠道。

## 官方依据

- Microsoft Learn：[Use SignTool to sign a file](https://learn.microsoft.com/en-us/windows/win32/seccrypto/using-signtool-to-sign-a-file)
- Microsoft Learn：[SignTool.exe](https://learn.microsoft.com/en-us/dotnet/framework/tools/signtool-exe)
- Microsoft Learn：[Get-AuthenticodeSignature](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.security/get-authenticodesignature)

## 回滚

移除签名步骤不会改变源代码业务行为；发布负责人仍必须重新生成未签名或旧签名包的 manifest，并运行 `verify-package.ps1`。不得把签名证书、私钥或正式用户数据放入回滚包。
