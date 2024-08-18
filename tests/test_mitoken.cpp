#include <iostream>

#include "mitoken/mitoken.h"

int main() {
    auto text = mitoken::load_utf8_text("./taylorswift.txt");
    auto tokens = mitoken::RegexTokenizer(text, R"( ?\w+)");
    tokens.train(256 + 1000);
//    tokens.print_dict();
//    std::cout << tokens.int_to_u8str(261) << std::endl;
    std::string s = "I'm sorry, the old Taylor can't come to the phone right now. Why? Oh, 'cause she's dead!";
    auto en = tokens.encode(s);
    for (auto const &val : en)
        std::cout << val << ", ";
    std::cout << std::endl;

    std::cout << (float)s.size() / en.size() << std::endl;

    std::cout << tokens.decode(en) << std::endl;
}