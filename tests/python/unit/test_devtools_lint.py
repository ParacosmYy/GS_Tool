"""devtools/lint.py 包装器单元测试 — main 退出码 + 参数构造。"""

from __future__ import annotations

from unittest.mock import patch

from embeddebug.devtools.lint import main


def test_main_returns_zero_on_clean():
    """ruff 通过时返回 0。"""
    with patch("subprocess.call", return_value=0):
        assert main([]) == 0


def test_main_returns_nonzero_on_errors():
    """ruff 有错误时返回非 0。"""
    with patch("subprocess.call", return_value=1):
        assert main([]) == 1


def test_main_default_args_include_check():
    """无参数时默认加 check + 路径。"""
    captured = []

    def fake_call(cmd, **kw):
        captured.extend(cmd)
        return 0

    with patch("subprocess.call", side_effect=fake_call):
        main([])
    assert "check" in captured
    assert "python/embeddebug" in captured


def test_main_explicit_check_not_duplicated():
    """显式传 check 不重复添加。"""
    captured = []

    def fake_call(cmd, **kw):
        captured.extend(cmd)
        return 0

    with patch("subprocess.call", side_effect=fake_call):
        main(["check", "--fix"])
    assert captured.count("check") == 1


def test_main_no_ruff_returns_2():
    """ruff 未安装时返回 2。"""
    import builtins
    real_import = builtins.__import__

    def fake_import(name, *args, **kwargs):
        if name == "ruff.__main__":
            raise ModuleNotFoundError("no ruff")
        return real_import(name, *args, **kwargs)

    with patch("builtins.__import__", side_effect=fake_import):
        assert main([]) == 2
