import os
import sys

def encode_dir_entries(directory, catalog_path):
    entries = []

    for root, dirs, files in os.walk(directory):
        root = root.encode('utf-8').decode('utf-8', 'ignore')  # UTF-8로 인코딩 및 디코딩
        entries.append((root, "DIR_START"))
        for name in dirs:
            name = name.encode('utf-8').decode('utf-8', 'ignore')
            entries.append((root, name))
        for name in files:
            name = name.encode('utf-8').decode('utf-8', 'ignore')
            entries.append((root, name))
        entries.append((root, "DIR_END"))

    with open(catalog_path, 'w', encoding='utf-8') as f:
        for root, name in entries:
            if name == "DIR_START":
                f.write(f"DIR_START {root}\n")
            elif name == "DIR_END":
                f.write("DIR_END\n")
            else:
                f.write(f"{name}\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <directory> <catalog_path>")
        sys.exit(1)

    directory = sys.argv[1]
    catalog_path = sys.argv[2]
    encode_dir_entries(directory, catalog_path)
    print(f"Catalog saved to {catalog_path}")
