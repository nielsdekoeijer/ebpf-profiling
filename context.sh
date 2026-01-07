#!/usr/bin/env bash

# Output filename
OUTPUT="context.txt"

# Check if this is a git repo
if [ ! -d .git ]; then
    echo "Error: This directory is not a git repository."
    exit 1
fi

# Clear or create the output file
> "$OUTPUT"

echo "Collecting tracked files into $OUTPUT..."

# List all files tracked by git
git ls-files | while read -r file; do
    
    # Skip the script itself and the output file to avoid recursion
    if [[ "$file" == "context.sh" || "$file" == "$OUTPUT" ]]; then
        continue
    fi

    # Check if the file is text (not binary) using grep
    # -I treats binary files as non-matching
    # -q suppresses output
    # '.' matches any character (so empty files are also skipped, which is usually good)
    if grep -Iq . "$file"; then
        echo "Processing: $file"
        
        # Write the file header
        echo "================================================================================" >> "$OUTPUT"
        echo "START OF FILE: $file" >> "$OUTPUT"
        echo "================================================================================" >> "$OUTPUT"
        
        # Write the file content
        cat "$file" >> "$OUTPUT"
        
        # Add a footer/spacing
        echo -e "\n\n" >> "$OUTPUT"
    else
        echo "Skipping binary or empty file: $file"
    fi
done

echo "Done! Content is ready in '$OUTPUT'"
