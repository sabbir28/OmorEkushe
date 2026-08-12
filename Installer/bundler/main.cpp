#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

void write_file_as_array(std::ofstream& out, const fs::path& path, const std::string& var_name) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return;

    out << "const unsigned char " << var_name << "[] = {";
    unsigned char buf[1024];
    size_t total = 0;
    while (in.read((char*)buf, sizeof(buf)) || in.gcount() > 0) {
        for (std::streamsize i = 0; i < in.gcount(); ++i) {
            if (total % 12 == 0) out << "\n    ";
            out << "0x" << std::hex << (int)buf[i] << ", ";
            total++;
        }
    }
    if (total == 0) {
        out << "0x00";
    }
    out << "\n};\n";
    out << "const size_t " << var_name << "_size = " << std::dec << total << ";\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: bundler <input_dir> <output_header>\n";
        return 1;
    }

    fs::path input_dir = argv[1];
    std::ofstream out(argv[2]);

    out << "#pragma once\n";
    out << "#include <cstddef>\n\n";
    out << "struct EmbeddedFile {\n";
    out << "    const char* path;\n";
    out << "    const unsigned char* data;\n";
    out << "    size_t size;\n";
    out << "};\n\n";

    std::vector<std::string> file_vars;
    std::vector<std::string> relative_paths;

    int file_idx = 0;
    if (fs::exists(input_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
            if (entry.is_regular_file()) {
                std::string var_name = "file_data_" + std::to_string(file_idx++);
                fs::path rel_path = fs::relative(entry.path(), input_dir);
                std::string rel_path_str = rel_path.string();
                std::replace(rel_path_str.begin(), rel_path_str.end(), '\\', '/');

                write_file_as_array(out, entry.path(), var_name);
                file_vars.push_back(var_name);
                relative_paths.push_back(rel_path_str);
            }
        }
    }

    if (file_vars.empty()) {
        out << "const unsigned char dummy_file_data[] = {0x00};\n";
        out << "const EmbeddedFile g_embedded_files[] = {\n";
        out << "    {\"dummy\", dummy_file_data, 0}\n";
        out << "};\n\n";
        out << "const size_t g_embedded_files_count = 0;\n";
    } else {
        out << "const EmbeddedFile g_embedded_files[] = {\n";
        for (size_t i = 0; i < file_vars.size(); ++i) {
            out << "    {\"" << relative_paths[i] << "\", " << file_vars[i] << ", " << file_vars[i] << "_size},\n";
        }
        out << "};\n\n";
        out << "const size_t g_embedded_files_count = " << file_vars.size() << ";\n";
    }

    return 0;
}
