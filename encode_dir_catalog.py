import os
import sys

def encode_dir_entries(directory, catalog_path):
    entries = []

    for root, dirs, files in os.walk(directory):
        for name in dirs + files:
            path = os.path.join(root, name)
            entries.append((root, name))

    entries.sort()

    with open(catalog_path, 'w') as f:
        for root, name in entries:
            f.write(f"{root}\n{name}\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <directory> <catalog_path>")
        sys.exit(1)

    directory = sys.argv[1]
    catalog_path = sys.argv[2]
    encode_dir_entries(directory, catalog_path)
    print(f"Catalog saved to {catalog_path}")
