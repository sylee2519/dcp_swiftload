import os

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
    directory = "/path/to/your/directory"  # Change this to the directory you want to encode
    catalog_path = "catalog_dir.txt"
    encode_dir_entries(directory, catalog_path)
    print(f"Catalog saved to {catalog_path}")
