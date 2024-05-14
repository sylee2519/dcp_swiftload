import os
import stat
import sys

def encode_stat_to_string(path, st):
    return f"{path}\n" \
           f"st_dev:{st.st_dev};" \
           f"st_ino:{st.st_ino};" \
           f"st_mode:{st.st_mode};" \
           f"st_nlink:{st.st_nlink};" \
           f"st_uid:{st.st_uid};" \
           f"st_gid:{st.st_gid};" \
           f"st_rdev:{st.st_rdev};" \
           f"st_size:{st.st_size};" \
           f"st_blksize:{st.st_blksize};" \
           f"st_blocks:{st.st_blocks};" \
           f"st_atime:{int(st.st_atime)};" \
           f"st_mtime:{int(st.st_mtime)};" \
           f"st_ctime:{int(st.st_ctime)}"

def encode_directory_to_catalog(directory, catalog_path):
    entries = []

    for root, dirs, files in os.walk(directory):
        for name in files + dirs:
            path = os.path.join(root, name)
            lstat_info = os.lstat(path)
            entry = f"lstat\n{encode_stat_to_string(path, lstat_info)}\n"
            if stat.S_ISLNK(lstat_info.st_mode):
                try:
                    stat_info = os.stat(path)
                    entry += f"stat\n{encode_stat_to_string(path, stat_info)}\n"
                except FileNotFoundError:
                    entry += "stat\nNone\n"
            else:
                entry += f"stat\n{encode_stat_to_string(path, lstat_info)}\n"
            entries.append(entry)

    entries.sort()

    with open(catalog_path, 'w') as f:
        for entry in entries:
            f.write(entry)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <directory> <catalog_path>")
        sys.exit(1)

    directory = sys.argv[1]
    catalog_path = sys.argv[2]
    encode_directory_to_catalog(directory, catalog_path)
    print(f"Catalog saved to {catalog_path}")
