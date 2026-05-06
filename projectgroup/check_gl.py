
import re

with open('Source.cpp', 'r') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    matches = re.finditer(r'glVertex3f\((.*?)\)', line)
    for match in matches:
        args = match.group(1).split(',')
        if len(args) != 3:
            print(f"Line {i+1}: {line.strip()} (Has {len(args)} arguments)")
