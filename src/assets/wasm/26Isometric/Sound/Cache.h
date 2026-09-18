#pragma once

#include <list>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <algorithm>

//https://codereview.stackexchange.com/questions/128560/caches-implementation-in-c
template <typename Key, typename Value>
class CacheLRU {
public:
	using value_type = typename std::pair<Key, Value>;
	using value_it = typename std::list<value_type>::iterator;

	CacheLRU() = default;
	CacheLRU(size_t max_size) : max_cache_size{ max_size } {
		if (max_size == 0) {
			max_cache_size = std::numeric_limits<size_t>::max();
		}
	}

	void Init(size_t max_size = 0) {
		if (max_size == 0) {
			max_cache_size = std::numeric_limits<size_t>::max();
		} else {
			max_cache_size = max_size;
		}
	}

	void Put(const Key& key, Value&& value) {
		auto it = cache_items_map.find(key);

		if (it == cache_items_map.end()) {
			if (cache_items_map.size() >= max_cache_size) {
				auto last = cache_items_list.crbegin();
				if constexpr (std::is_pointer_v<Value>) {
					delete last->second;
				}
				cache_items_map.erase(last->first);
				cache_items_list.pop_back();
			}

			cache_items_list.emplace_front(key, std::move(value));
			cache_items_map[key] = cache_items_list.begin();
		} else {
			it->second->second = std::move(value);
			cache_items_list.splice(cache_items_list.begin(), cache_items_list, it->second);
		}
	}

	void Put(const Key& key) {
		auto it = cache_items_map.find(key);

		if (it == cache_items_map.end()) {
			if (cache_items_map.size() >= max_cache_size) {
				auto last = cache_items_list.crbegin();
				if constexpr (std::is_pointer_v<Value>) {
					delete last->second;
				}
				cache_items_map.erase(last->first);
				cache_items_list.pop_back();
			}

			cache_items_list.emplace_front(key, key);
			cache_items_map[key] = cache_items_list.begin();

		} else {
			cache_items_list.splice(cache_items_list.begin(), cache_items_list, it->second);
		}
	}

	const Value& Get(const Key& key) {
		auto it = cache_items_map.find(key);

		if (it == cache_items_map.end()) {
			if (cache_items_map.size() + 1 > max_cache_size) {
				auto last = cache_items_list.crbegin();
				if constexpr (std::is_pointer_v<Value>) {
					delete last->second;
				}
				cache_items_map.erase(last->first);
				cache_items_list.pop_back();
			}
		
			cache_items_list.emplace_front(key, key);
			cache_items_map[key] = cache_items_list.begin();

			return cache_items_list.front().second;
		} else {
			cache_items_list.splice(cache_items_list.begin(), cache_items_list, it->second);

			return cache_items_list.front().second;
		}
	}

	bool Exists(const Key& key) const noexcept {
		return cache_items_map.find(key) != cache_items_map.end();
	}

	size_t Size() const noexcept {
		return cache_items_map.size();
	}

	void Clear() noexcept {
		if constexpr (std::is_pointer_v<Value>) {
			for (const auto& item : cache_items_list) {
				delete item.second;
			}
		}
		cache_items_map.clear();
		cache_items_list.clear();
	}

private:

	mutable std::list<value_type> cache_items_list;
	std::unordered_map<Key, value_it> cache_items_map;
	size_t max_cache_size = 10u;
};