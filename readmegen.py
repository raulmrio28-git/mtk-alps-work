import os
from pathlib import Path

def parse_directories_pathlib(start_dir):
    p = Path(start_dir)
    paths = []
    all_paths = p.rglob("*")

    for path in all_paths:
        if ".git" not in path.parts and ".obsidian" not in path.parts and "image" not in path.parts and path.is_dir():
            paths.append(path)
    return paths

def parse_folder(folder, roota):
    out = ""
    ### get folder name
    folder_name = os.path.basename(folder)
    out += "# " + folder_name + "\n\n"
    #list folder
    for root, dirs, files in os.walk(folder):
        for file in files:
            if file.endswith(".md"):
                ######converted path to /
                path = os.path.join(root, file)
                path = path.replace(roota, "").replace("\\", "/")
                out += "* [" + file.replace(".md", "") + "](" + path + ")\n\n"
    return out

if __name__ == "__main__":
    executed = os.path.dirname(os.path.abspath(__file__))
    ###perse each folder
    paths = parse_directories_pathlib(executed)
    with open("README.md", "w") as f:
        for path in paths:
            f.write(parse_folder(str(path), executed))
        
