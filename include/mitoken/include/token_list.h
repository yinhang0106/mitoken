#pragma once

#include "core.h"

namespace mitoken::details {

    struct Token {
        Token *m_prev;
        Token *m_next;
        token_int m_value;
    };

    struct TokenList {
        Token *m_data;
        size_t m_size;
        stats_type m_stats{};
        Token *m_dummy = nullptr;

        TokenList() noexcept
                : m_data(nullptr), m_size(0) {}

        TokenList(std::initializer_list<token_int> ilist)
                : TokenList(ilist.begin(), ilist.end()) {}

        template<std::random_access_iterator InputIt>
        TokenList(InputIt first, InputIt last) {
            size_t n = last - first;
            m_data = new Token[n];
            m_size = n;

            auto ptr = m_data;
            ptr->m_prev = nullptr;
            for (size_t i = 0; i < n - 1; i++) {
                ptr->m_next = ptr + 1;
                (ptr + 1)->m_prev = ptr;
                ptr->m_value = *first;
                ++first;
                ptr = ptr->m_next;
            }
            ptr->m_next = nullptr;
            ptr->m_value = *first;

            // calculate statistics
            init_stats();
        }

        ~TokenList() { delete[] m_data; }

        void init_stats() {
            m_stats.clear();
            auto ptr = m_data;
            for (size_t i = 0; i < m_size - 1; i++) {
                m_stats.insert_or_assign({ptr->m_value, ptr->m_next->m_value},
                                         m_stats[{ptr->m_value, ptr->m_next->m_value}] + 1);
                ptr = ptr->m_next;
            }
        }

        struct iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = token_int;
            using difference_type = std::ptrdiff_t;
            using pointer = token_int *;
            using reference = token_int &;

            Token *m_curr;

            iterator &operator++() { // ++iterator
                m_curr = m_curr->m_next;
                return *this;
            }

            iterator operator++(int) { // iterator++
                iterator tmp = *this;
                m_curr = m_curr->m_next;
                return tmp;
            }

            reference operator*() const {
                return m_curr->m_value;
            }

            bool operator!=(iterator const &that) const {
                return m_curr != that.m_curr;
            }

            bool operator==(iterator const &that) const {
                return !(*this != that);
            }
        };

        [[nodiscard]]
        Token * data() const noexcept { return m_data; }

        [[nodiscard]]
        iterator begin() const { return iterator{m_data}; }

        [[nodiscard]]
        iterator end() const { return iterator{m_dummy}; }

        [[nodiscard]]
        size_t size() const noexcept { return m_size; }

        [[nodiscard]] typename
        iterator::reference operator[](size_t index) const {
            auto ptr = m_data;
            for (size_t i = 0; i < index; i++)
                ptr = ptr->m_next;
            return ptr->m_value;
        }

        [[nodiscard]] typename
        iterator::reference at(size_t index) const {
            if (index >= m_size)
                [[unlikely]] throw std::out_of_range("TokenList::at");
            return operator[](index);
        }

        void merge(pair_type &val, token_int new_val) {
            auto key = val.first;
            auto next = val.second;
            auto ptr = m_data;

            while (ptr->m_next != nullptr) {
                if (ptr->m_value == key && ptr->m_next->m_value == next) {
                    // update stats
                    // first fix the old stats
                    m_stats.erase({key, next});
                    if (ptr->m_prev != nullptr)
                        m_stats[{ptr->m_prev->m_value, key}]--;
                    if (ptr->m_next->m_next != nullptr) {
                        if (m_stats[{next, ptr->m_next->m_next->m_value}] > 1)
                            m_stats[{next, ptr->m_next->m_next->m_value}]--;
                        else
                            m_stats.erase({next, ptr->m_next->m_next->m_value});
                    }

                    // update values
                    ptr->m_value = new_val;
                    ptr->m_next = ptr->m_next->m_next;
                    if (ptr->m_next != nullptr)
                        ptr->m_next->m_prev = ptr;
                    m_size--;

                    // update stats
                    if (ptr->m_next != nullptr)
                        m_stats.insert_or_assign({ptr->m_value, ptr->m_next->m_value},
                                                 m_stats[{ptr->m_value, ptr->m_next->m_value}] + 1);

                    if (ptr->m_prev != nullptr)
                        m_stats.insert_or_assign({ptr->m_prev->m_value, ptr->m_value},
                                                 m_stats[{ptr->m_prev->m_value, ptr->m_value}] + 1);
                }
                ptr = ptr->m_next;
            }
        }

    };

} // namespace mitoken::details
