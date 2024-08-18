#include <fstream>
#include <utf8.h>


namespace mitoken {

    using text_type = std::string;

    // make sure the file is utf-8 encoded
    inline text_type load_utf8_text(const std::string& file_name) {
        std::ifstream ifs(file_name);
        if (!ifs.is_open()) {
            throw std::runtime_error("Error: file is not open!");
        }

        unsigned line_count = 1;
        text_type text;
        text_type line;
        while (getline(ifs, line)) {
            // check for invalid utf-8 (for a simple yes/no check, there is also utf8::is_valid function)
            auto end_it = utf8::find_invalid(line.begin(), line.end());
            if (end_it != line.end()) {
                throw std::runtime_error("Error: invalid utf-8 in line " + std::to_string(line_count));
            }
            text += (std::string(line.begin(), end_it) + "\n");
        }
        return text;
    }

    [[maybe_unused]]
    inline bool valid_utf8_file(const std::string& file_name) {
        std::ifstream ifs(file_name);
        if (!ifs.is_open())
            throw std::runtime_error("Error: file is not open!");

        std::istreambuf_iterator<char> it(ifs.rdbuf());
        std::istreambuf_iterator<char> eos;

        return utf8::is_valid(it, eos);
    }

    [[maybe_unused]]
    inline void fix_utf8_string(std::string& str) {
        std::string temp;
        utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
        str = temp;
    }

    [[maybe_unused]]
    inline std::u16string utf8to16(const std::string& str) {
        return utf8::utf8to16(str);
    }

    [[maybe_unused]]
    inline std::string utf16to8(const std::u16string& str) {
        return utf8::utf16to8(str);
    }

    [[maybe_unused]]
    inline std::u32string utf8to32(const std::string& str) {
        return utf8::utf8to32(str);
    }

    [[maybe_unused]]
    inline std::string utf32to8(const std::u32string& str) {
        return utf8::utf32to8(str);
    }

} // namespace mitoken::utf