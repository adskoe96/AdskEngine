#pragma once
#include <string>
#include <vector>
#include <map>

struct file_info
{
    std::string name;
    std::string path;
    int size;
    int uncompressed_size;
    int start_position;
    int end_position;
};

struct pack_info
{
    std::string name;
    std::string path;
    std::map<std::string, file_info> files;
};

class wwp
{
public:
    static void set_packs_directory(const std::string& path);
    static std::vector<pack_info> get_all_packs();
    static bool pack_files(const std::string& source, const std::string& name);
    static bool unpack_files(const std::string& destination, pack_info& pack);
    static bool unpack_file(const std::string& destination, const std::string file, pack_info& pack);
    static std::vector<char> read_file(const std::string& file, pack_info& pack, bool into_cache = false);
    static void clear_cache();
    static void remove_from_cache(const std::string& file);
    static bool has_file(const std::string& file, pack_info& pack);
};
