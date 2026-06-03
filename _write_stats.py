#!/usr/bin/env python3
"""Temporary script to write updated stats files."""
import os, sys

def write_file(path, content):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(content)
    print(f"Written: {path} ({len(content.splitlines())} lines)")

BASE = r"E:\Embedded\Tool\Serial_tool\User_Serial\src"

# Read original files to get the Chinese text correctly
orig_bridge_cpp = open(os.path.join(BASE, "protocol", "bridge", "ProtocolBridgeManager.cpp"), "r", encoding="utf-8").read()
orig_parser_cpp = open(os.path.join(BASE, "parser", "FrameParser.cpp"), "r", encoding="utf-8").read()
orig_statehandlers_cpp = open(os.path.join(BASE, "parser", "FrameParserStateHandlers.cpp"), "r", encoding="utf-8").read()
orig_modbus_cpp = open(os.path.join(BASE, "protocol", "modbus", "ModbusMaster.cpp"), "r", encoding="utf-8").read()

# We already wrote the .h files correctly with the Write tool.
# Now we need to write the .cpp files with the correct Chinese text.

# Just verify the .h files are correct
print("Verifying .h files...")
h_bridge = open(os.path.join(BASE, "protocol", "bridge", "ProtocolBridgeManager.h"), "r", encoding="utf-8").read()
h_parser = open(os.path.join(BASE, "parser", "FrameParser.h"), "r", encoding="utf-8").read()
h_modbus = open(os.path.join(BASE, "protocol", "modbus", "ModbusMaster.h"), "r", encoding="utf-8").read()

print(f"ProtocolBridgeManager.h: {len(h_bridge.splitlines())} lines")
print(f"FrameParser.h: {len(h_parser.splitlines())} lines")
print(f"ModbusMaster.h: {len(h_modbus.splitlines())} lines")

# Check for unicode escapes in .h files
for name, content in [("ProtocolBridgeManager.h", h_bridge), ("FrameParser.h", h_parser), ("ModbusMaster.h", h_modbus)]:
    if "\\u" in content:
        print(f"WARNING: {name} contains \\u escapes!")
    else:
        print(f"OK: {name} has no unicode escapes")
