import ast, os, re

roots = ["src/serialforge"]
files = []
for r in roots:
    for dp, _, fs in os.walk(r):
        for f in fs:
            if f.endswith(".py"):
                files.append(os.path.join(dp, f))

stats = dict(files=0, no_mod_doc=0, pub_func_no_doc=0, no_ret_annot=0,
             bare_except=0, open_write_no_enc=0)


def has_value_return(fnode):
    for n in ast.walk(fnode):
        if isinstance(n, ast.Return) and n.value is not None:
            return True
        if isinstance(n, (ast.Yield, ast.YieldFrom)):
            return True
    return False


for fp in files:
    stats["files"] += 1
    src = open(fp, encoding="utf-8").read()
    tree = ast.parse(src)
    lines = src.splitlines()
    if not (tree.body and isinstance(tree.body[0], ast.Expr)
            and isinstance(tree.body[0].value, ast.Constant)
            and isinstance(tree.body[0].value.value, str)):
        stats["no_mod_doc"] += 1
    for node in ast.walk(tree):
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            has_doc = ast.get_docstring(node)
            if not node.name.startswith("_") and not has_doc:
                stats["pub_func_no_doc"] += 1
            if node.returns is None and not has_value_return(node):
                stats["no_ret_annot"] += 1
    for ln in lines:
        s = ln.lstrip()
        if re.match(r'^except\s*:\s*(#.*)?$', s):
            stats["bare_except"] += 1
    for m in re.finditer(r'open\s*\(([^)]*)\)', src):
        arg = m.group(1)
        if 'encoding=' in arg:
            continue
        if re.search(r"mode\s*=\s*['\"][^'\"]*b", arg):
            continue
        mm = re.search(r"['\"]([rwaxt+]*)['\"]", arg)
        if mm and 'b' in mm.group(1):
            continue
        stats["open_write_no_enc"] += 1

print(stats)
print("SUM safe-ish:", stats["no_mod_doc"] + stats["pub_func_no_doc"]
      + stats["no_ret_annot"] + stats["bare_except"] + stats["open_write_no_enc"])
