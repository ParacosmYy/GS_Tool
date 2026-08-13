import ast, os

roots = ["src/serialforge"]
files = []
for r in roots:
    for dp, _, fs in os.walk(r):
        for f in fs:
            if f.endswith(".py"):
                files.append(os.path.join(dp, f))

total_func = 0
missing_doc = 0
missing_doc_pub = 0
missing_doc_pub_modlevel = 0
classes = 0
for fp in files:
    src = open(fp, encoding="utf-8").read()
    tree = ast.parse(src)

    def walk_parent(node):
        # determine if a func def is module-level
        pass

    for node in ast.walk(tree):
        if isinstance(node, ast.ClassDef):
            classes += 1
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)):
            total_func += 1
            has_doc = ast.get_docstring(node)
            if not has_doc:
                missing_doc += 1
                if not node.name.startswith("_"):
                    missing_doc_pub += 1
                    # module level?
                    if not any(isinstance(p, ast.ClassDef) for p in ast.walk(tree)
                               if False):
                        pass

print("files:", len(files))
print("classes:", classes)
print("total functions/methods:", total_func)
print("missing docstring (all):", missing_doc)
print("missing docstring (public only):", missing_doc_pub)
PY
