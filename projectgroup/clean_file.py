
with open('Source.cpp', 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

# Replace common non-ASCII separators with ASCII
content = content.replace('───', '---')
content = content.replace('──', '--')
content = content.replace('─', '-')
content = content.replace('│', '|')

# Remove any other non-ASCII characters
content = "".join([c if ord(c) < 128 else " " for c in content])

with open('Source.cpp', 'w', encoding='ascii') as f:
    f.write(content)
