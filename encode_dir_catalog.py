import os
import sys

def encode_dir_entries(directory, catalog_path):
    entries = []

    for root, dirs, files in os.walk(directory):
        for name in dirs + files:
            path = os.path.join(root, name)
            entry_type = "D" if os.path.isdir(path) else "F"
            entries.append((root, name, entry_type))

    entries.sort()

    with open(catalog_path, 'w') as f:
        prev_root = None
        for root, name, entry_type in entries:
            if root != prev_root:
                if prev_root is not None:
                    f.write("DIR_END\n")
                f.write(f"DIR_START {root}\n")
                prev_root = root
            f.write(f"{name} {entry_type}\n")
        if prev_root is not None:
            f.write("DIR_END\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <directory> <catalog_path>")
        sys.exit(1)

    directory = sys.argv[1]
    catalog_path = sys.argv[2]
    encode_dir_entries(directory, catalog_path)
    print(f"Catalog saved to {catalog_path}")
