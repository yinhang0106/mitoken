#pragma once

#include <regex>
#include <format>
#include <stdexcept>
#include <algorithm>

#define UTF8_BYTE_SIZE 256

namespace mitoken {

    using token_list_t = std::vector<std::vector<int>>;
    using stats_t = std::map<std::pair<int, int>, int>;
    using merges_t = std::map<std::pair<int, int>, int>;
    using dict_t = std::map<int, std::pair<int, int>>;

    struct RegexTokenizer {
        token_list_t    m_tokens;
        stats_t         m_stats;
        merges_t        m_merges;
        dict_t          m_dict;

        RegexTokenizer(const std::string& text, const std::string& pattern) {
            _from_regex_to_tokens(text, pattern);
            _init_stats();
        }

        // help functions
        void _from_regex_to_tokens(const std::string& text, const std::string& pattern) {
            std::regex re(pattern);
            std::sregex_iterator it(text.begin(), text.end(), re);
            std::sregex_iterator end;

            while (it != end) {
                std::smatch match = *it++;
                std::vector<int> tokens;
                for (auto const & ch : match.str()) {
                    tokens.push_back(static_cast<unsigned char>(ch));
                }
                m_tokens.push_back(tokens);
            }
        }

        void _init_stats() {
            for (auto const& tokens : m_tokens) {
                for (size_t i = 0; i < tokens.size() - 1; i++) {
                    m_stats.insert_or_assign({tokens[i], tokens[i + 1]},
                                             m_stats[{tokens[i], tokens[i + 1]}] + 1);
                }
            }
        }

        void _merge_most_frequent_pair(int new_token_id) {
            auto max_it = std::max_element(m_stats.begin(), m_stats.end(),
                                           [](const auto& a, const auto& b) {
                                               return a.second < b.second;
                                           });
            if (max_it->second == 1) return;

            std::cout << "Most frequent pair: " << max_it->first.first << " "
                      << max_it->first.second << " " << max_it->second << "\n";

            m_merges.insert_or_assign(max_it->first, new_token_id);

            // merge the most frequent pair

            m_stats.erase(max_it->first);

            for (auto& tokens : m_tokens) {
                for (size_t i = 0; i < tokens.size() - 1; i++) {
                    if (tokens[i] == max_it->first.first && tokens[i + 1] == max_it->first.second) {
                        // stream update stats
                        if (i > 0) {
                            m_stats[{tokens[i - 1], tokens[i]}]--;
                            if (m_stats[{tokens[i - 1], tokens[i]}] == 0) {
                                m_stats.erase({tokens[i - 1], tokens[i]});
                            }
                        }
                        if (i < tokens.size() - 2) {
                            m_stats[{tokens[i + 1], tokens[i + 2]}]--;
                            if (m_stats[{tokens[i + 1], tokens[i + 2]}] == 0) {
                                m_stats.erase({tokens[i + 1], tokens[i + 2]});
                            }
                        }

                        // update values
                        tokens[i] = new_token_id;
                        tokens.erase(tokens.begin() + i + 1);

                        // stream update stats
                        if (i > 0) {
                            m_stats.insert_or_assign({tokens[i - 1], tokens[i]},
                                                     m_stats[{tokens[i - 1], tokens[i]}] + 1);

                        }
                        if (i < tokens.size() - 1) {
                            m_stats.insert_or_assign({tokens[i], tokens[i + 1]},
                                                     m_stats[{tokens[i], tokens[i + 1]}] + 1);
                        }
                    }
                }
            }

        }

        void _from_merge_to_dict() {
            for (const auto& it : m_merges)
                m_dict.insert_or_assign(it.second, it.first);
        }

        void train(int vocab_size) {
            if (vocab_size < UTF8_BYTE_SIZE)
                throw std::invalid_argument(std::format("vocab_size must be greater than {}", UTF8_BYTE_SIZE));

            for (int i = 0; i < vocab_size - UTF8_BYTE_SIZE; i++) {
                _merge_most_frequent_pair(UTF8_BYTE_SIZE + i);
            }

            _from_merge_to_dict();
        }


        void print_tokens() {
            for (const auto& tokens : m_tokens) {
                for (const auto& t : tokens)
                    std::cout << t << ' ';
                std::cout << '\n';
            }
        }

        void print_stats() {
            for (const auto& it : m_stats)
                std::cout << it.first.first << " " << it.first.second << " " << it.second << "\n";
        }

        void print_merges() {
            for (const auto& it : m_merges)
                std::cout << std::format("merge ({:>5d}, {:>5d}) -> {:>5d}\n", it.first.first, it.first.second, it.second);
        }

        void print_dict() {
            for (const auto& it : m_dict)
                std::cout << std::format("dict {:>5d} -> ({:>5d}, {:>5d})\n", it.first, it.second.first, it.second.second);
        }

        std::string int_to_u8str(int val) {
            if (val > UTF8_BYTE_SIZE - 1) {
                auto it = m_dict.find(val);
                return int_to_u8str(it->second.first) + int_to_u8str(it->second.second);
            }
            return {static_cast<char>(val)};
        }

        std::string decode(std::vector<int>& vec) {
            std::string str;
            for (const auto& val : vec)
                str += int_to_u8str(val);
            return str;
        }

        std::vector<int> encode(const std::string& str) {
            std::vector<int> tokens;
            for (auto const &c: str) {
                tokens.emplace_back((unsigned char) (c));
            }
            for (int i = 0; i < tokens.size() - 1; i++) {
                while (m_merges.find({tokens[i], tokens[i + 1]}) != m_merges.end()) {
                    tokens[i] = m_merges[{tokens[i], tokens[i + 1]}];
                    tokens.erase(tokens.begin() + i + 1);
                }
            }
            return tokens;
        }

    };

}   // namespace mitoken