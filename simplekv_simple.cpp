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

/*
 * simplekv_word_count.cpp -- implementation of a map-reduce algorithm
 * for counting words in text files.
 *
 * create the pool for this program using pmempool, for example:
 *	pmempool create obj --layout=simplekv -s 1G word_count
 */

#include "simplekv.hpp"

static const std::string LAYOUT = "simplekv";

using pmem::obj::delete_persistent;
using pmem::obj::make_persistent;
using pmem::obj::p;
using pmem::obj::persistent_ptr;
using pmem::obj::pool;
using pmem::obj::transaction;

namespace ptl = pmem::obj::experimental;

using simplekv_type = examples::kv<int, int, 10>;

struct root {
	persistent_ptr<simplekv_type> simplekv;
};

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " file-name" << std::endl;
		return 1;
	}

	auto path = argv[1];

	auto pop = pool<root>::open(path, LAYOUT);
	auto r = pop.root();

	if (r->simplekv != nullptr) {
		transaction::run(pop, [&]() {
			delete_persistent<simplekv_type>(r->simplekv);
		});
	}

	transaction::run(pop, [&]() {
		r->simplekv = make_persistent<simplekv_type>();
	});

	r->simplekv->insert(1, 1);
	r->simplekv->insert(2, 3);
	r->simplekv->insert(3, 4);
	r->simplekv->insert(11, 11);

	assert(r->simplekv->at(1) == 1);
	assert(r->simplekv->at(2) == 3);
	assert(r->simplekv->at(3) == 4);
	assert(r->simplekv->at(11) == 11);

	pop.close();

	return 0;
}
