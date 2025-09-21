#!/usr/bin/env python3
"""Add a new PPAT8 pattern with ID 304 to patterns.json"""

import json

# Read the existing patterns
with open('patterns.json', 'r') as f:
    data = json.load(f)

# Create the new pattern 304 - BluePixel pattern inspired by the image
new_pattern = {
    "type": "ppat",
    "id": 304,
    "name": "BluePixel",
    "data": {
        "ppat8": {
            "palette": [
                [96, 175, 125, 255],     # Green
                [229, 229, 229, 255],    # Light gray
                [160, 160, 160, 255],    # Medium gray
                [255, 255, 255, 255]     # White
            ],
            "indices": [
                "3 1 0 0 0 0 1 3",
                "1 2 0 0 0 0 2 1",
                "0 0 1 3 3 1 0 0",
                "0 0 3 2 2 3 0 0",
                "0 0 3 2 2 3 0 0",
                "0 0 1 3 3 1 0 0",
                "1 2 0 0 0 0 2 1",
                "3 1 0 0 0 0 1 3"
            ]
        }
    }
}

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

print("Added pattern 304 (BluePixel) to patterns.json")