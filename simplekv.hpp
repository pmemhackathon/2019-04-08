/*
 * Copyright 2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <bitset>
#include <iostream>
#include <libpmemobj++/experimental/array.hpp>
#include <libpmemobj++/experimental/string.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/mutex.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pext.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>
#include <stdexcept>
#include <string>

namespace examples
{

namespace ptl = pmem::obj::experimental;

template <typename T>
struct hash;

template <>
struct hash<ptl::string> {
	std::size_t
	operator()(const ptl::string &data, int n)
	{
		assert(n <= 1);

		static constexpr std::size_t params[] = {
			0xff51afd7ed558ccd,
			0xc4ceb9fe1a85ec53,
			0x5fcdfd7ed551af8c,
			0xec53ba85e9fe1c4c,
		};
		std::string str(data.cbegin(), data.cend());
		std::size_t key = std::hash<std::string>{}(str);
		key ^= key >> 33;
		key *= params[n * 2];
		key ^= key >> 33;
		key *= params[(n * 2) + 1];
		key ^= key >> 33;
		return key;
	}
};

template <>
struct hash<uint64_t> {
	std::size_t
	operator()(const uint64_t &data, int n)
	{
		assert(n <= 1);

		static constexpr std::size_t params[] = {
			0xff51afd7ed558ccd,
			0xc4ceb9fe1a85ec53,
			0x5fcdfd7ed551af8c,
			0xec53ba85e9fe1c4c,
		};

		std::size_t key = data;
		key ^= data >> 33;
		key *= params[n * 2];
		key ^= key >> 33;
		key *= params[(n * 2) + 1];
		key ^= key >> 33;
		return key;
	}
};

using pmem::obj::delete_persistent;
using pmem::obj::make_persistent;
using pmem::obj::mutex;
using pmem::obj::p;
using pmem::obj::persistent_ptr;
using pmem::obj::pool;
using pmem::obj::pool_base;
using pmem::obj::transaction;

/**
 * HashFunc should implement operator(const Key& k, int n)
 * which calculates hash function based on key and some integer.
 */
template <typename Key, typename Value, std::size_t N,
	  typename HashFunc = hash<Key>>
class kv {
private:
	struct slot;
	struct entry;

	static const int nretries = 5;
	static const int nhash = 2;

	ptl::array<slot, N> slots[nhash];
	ptl::vector<entry> entries;

public:
	using value_type = entry;

	kv() = default;

	Value &
	at(const Key &k)
	{
		/* TODO */

		throw std::runtime_error("not implemented");
	}

	void
	insert(const Key &key, const Value &val)
	{
		/* TODO */

		throw std::runtime_error("not implemented");
	}

	auto
	begin() -> decltype(entries.begin())
	{
		return entries.begin();
	}

	auto
	end() -> decltype(entries.end())
	{
		return entries.end();
	}

	auto
	begin() const -> decltype(entries.cbegin())
	{
		return entries.cbegin();
	}

	auto
	end() const -> decltype(entries.cend())
	{
		return entries.cend();
	}

	auto
	cbegin() const -> decltype(entries.cbegin())
	{
		return entries.cbegin();
	}

	auto
	cend() const -> decltype(entries.cend())
	{
		return entries.cend();
	}

private:
	size_t
	key_hash(const Key &k, int n) const
	{
		return HashFunc{}(k, n) & (N - 1);
	}

	pool_base
	get_pool() const noexcept
	{
		auto pop = pmemobj_pool_by_ptr(this);
		assert(pop != nullptr);
		return pool_base(pop);
	}

	struct entry {
		entry(const Key &k = 0, const Value &v = Value{})
		    : key(k), value(v)
		{
		}

		Key key;
		Value value;
	};

	struct slot {
		slot() : occupied(false), index(0)
		{
		}

		void
		set(std::size_t index)
		{
			this->index = index;
			occupied = true;
		}

		bool occupied;
		std::size_t index;
	};
};

} /* namespace examples */
