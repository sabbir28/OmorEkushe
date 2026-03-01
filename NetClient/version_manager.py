import os
import sys

def get_next_version(current):
    major, minor = map(int, current.split('.'))
    minor += 1
    if minor >= 10:
        major += 1
        minor = 0
    return f"{major}.{minor}"

def generate_rc(version, company, description):
    v_parts = version.split('.')
    v_comma = f"{v_parts[0]},{v_parts[1]},0,0"
    
    rc_content = f"""
#include <windows.h>

VS_VERSION_INFO VERSIONINFO
 FILEVERSION {v_comma}
 PRODUCTVERSION {v_comma}
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "{company}"
            VALUE "FileDescription", "{description}"
            VALUE "FileVersion", "{version}.0.0"
            VALUE "InternalName", "NetClient.dll"
            VALUE "LegalCopyright", "Copyright (C) 2026 {company}"
            VALUE "OriginalFilename", "NetClient.dll"
            VALUE "ProductName", "NetClient SDK"
            VALUE "ProductVersion", "{version}.0.0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
"""
    return rc_content

def main():
    version_file = "VERSION"
    if not os.path.exists(version_file):
        with open(version_file, "w") as f: f.write("0.1")
    
    with open(version_file, "r") as f:
        current_version = f.read().strip()
    
    # We only increment on explicit request or as part of a post-build step in CMake
    # For now, let's just use the current version to generate the files.
    # The user wanted "atuomatick vertion numberr update system"
    next_version = get_next_version(current_version)
    
    # Update VERSION file for next time
    with open(version_file, "w") as f:
        f.write(next_version)
        
    company = "OmorEkushe Team"
    description = "NetClient Network Library for OmorEkushe"
    
    rc_path = "src/NetClient.rc"
    with open(rc_path, "w") as f:
        f.write(generate_rc(next_version, company, description))

    header_content = f"""
#ifndef NETCLIENT_VERSION_H
#define NETCLIENT_VERSION_H

#define NETCLIENT_VERSION "{next_version}"
#define NETCLIENT_COMPANY "{company}"

#endif
"""
    with open("include/NetClientVersion.h", "w") as f:
        f.write(header_content)
    
    print(f"Version updated to {next_version}")

if __name__ == "__main__":
    # Change CWD to the script directory if needed, but we expect to run from NetClient/
    main()
