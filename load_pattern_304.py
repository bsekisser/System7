#!/usr/bin/env python3
"""Load pattern from JSON file and add it as pattern 304"""

import json

# Load the pattern from the provided JSON file
with open('/home/k/Documents/pattern_thisone.json', 'r') as f:
    source = json.load(f)

# Get the pattern data
source_pattern = source['resources'][0]

# Read the existing patterns
with open('patterns.json', 'r') as f:
    data = json.load(f)

# The source pattern contains raw PPAT8 data in "bytes" format
# We can use it as-is since gen_rsrc.py now handles "bytes" format
new_pattern = {
    "type": "ppat",
    "id": 304,
    "name": "PatternThisOne",
    "data": source_pattern['data']  # Contains "bytes" with PPAT8 format data
}

# Remove any existing pattern 304
data['resources'] = [r for r in data['resources'] if not (r.get('type') == 'ppat' and r.get('id') == 304)]

# Find where to insert it (after ppat 302)
inserted = False
for i, resource in enumerate(data['resources']):
    if resource.get('type') == 'ppat' and resource.get('id') == 302:
        # Insert after pattern 302
        data['resources'].insert(i + 1, new_pattern)
        inserted = True
        break

# If pattern 302 wasn't found, just append it
if not inserted:
    data['resources'].append(new_pattern)

# Write back the updated patterns
with open('patterns.json', 'w') as f:
    json.dump(data, f, indent=2)

print("Added pattern 304 (PatternThisOne) to patterns.json")