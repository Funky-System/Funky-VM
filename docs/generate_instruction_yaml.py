import fnmatch
import os
import re

docsdir = os.path.dirname(os.path.abspath(__file__))

output = open("instructions.yaml", "w")


def process(match):
    comment = match[0]
    comment = re.sub(r'/\*\*!', '', comment)
    comment = re.sub(r'\*/', '', comment)
    comment = re.sub(r'\n\s*\*', '\n', comment)
    comment = re.sub(r'(\n|^)\s*\n', '', comment)
    trim_spaces = len(comment) - len(comment.lstrip())
    comment = re.sub(r'(?<=\n)\s{' + str(trim_spaces) + '}', '', comment)
    comment = re.sub(r'(?<=^)\s{' + str(trim_spaces) + '}', '', comment)
    comment = re.sub(r'\n', '\n  ', comment)
    comment = "- " + comment
    output.write(comment)
    output.write("\n")


source_files = []
for root, dirnames, filenames in os.walk(docsdir + '/../src'):
    for filename in fnmatch.filter(filenames, '*.c'):
        source_files.append(os.path.join(root, filename))

for source_file in source_files:
    fp = open(source_file, "r")
    contents = fp.read()
    matches = re.findall(r"(/\*\*!.*?\*/)\s*INSTR\(([a-zA-Z0-9_]+)\)", contents, re.MULTILINE | re.DOTALL)
    for match in matches:
        process(match)
    fp.close()

output.close()
