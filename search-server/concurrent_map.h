#pragma once

#include <vector>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <execution>
#include <mutex>
#include <atomic>

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    explicit ConcurrentMap(size_t bucket_count) :bucket_count_(bucket_count) {
        vector_value_ = std::vector<Bucket>(bucket_count_);
    };

    struct Bucket {
        std::map<Key, Value > map_val;
        std::mutex mut;
    };

    struct Access {
        std::lock_guard<std::mutex> guard_;
        Value& ref_to_value;
        Access(const Key& key, std::map<Key, Value>& all_map, std::mutex& mx) : guard_(mx), ref_to_value(all_map[key]) {
        }
    };

    //работает как MAP возвращая значение по ключу
    Access operator[](const Key& key) {
        std::atomic_int i = static_cast<uint64_t>(key) % bucket_count_;
        return Access(key, vector_value_[i].map_val, vector_value_[i].mut);
    };

    void erase(const Key& key) {
        std::lock_guard<std::mutex> guard_(vector_value_[static_cast<uint64_t>(key) % bucket_count_].mut);
        vector_value_[static_cast<uint64_t>(key) % bucket_count_].map_val.erase(key);
    }

    //собирает массив воедино
    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& map_in : vector_value_) {
            std::lock_guard<std::mutex> guard_(map_in.mut);
            for (const auto& [key, value] : map_in.map_val) {
                result[key] = value;
            }
        };
        return result;
    };

private:
    std::uint64_t bucket_count_;
    std::vector<Bucket>vector_value_;
};