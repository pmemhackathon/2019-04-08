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
 * queue_pmemobj.cpp -- implementation of a persistent queue.
 *
 * create the pool for this program using pmempool, for example:
 *	pmempool create obj --layout=queue -s 1G queue_pool
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <libpmemobj.h>

POBJ_LAYOUT_BEGIN(queue);
POBJ_LAYOUT_TOID(queue, struct queue);
POBJ_LAYOUT_TOID(queue, struct queue_node);
POBJ_LAYOUT_END(queue);

enum queue_op {
	PUSH,
	POP,
	SHOW,
	MAX_OPS,
};

struct queue_node {
	int value;
	TOID(struct queue_node) next;
};

struct queue {
	void
	push(PMEMobjpool *pop, int value)
	{
		TX_BEGIN(pop) {
			TOID(struct queue_node) node = TX_ALLOC(struct queue_node,
                                                    sizeof(struct queue_node));
			D_RW(node)->value = value;
			D_RW(node)->next = TOID_NULL(struct queue_node);

			if (TOID_IS_NULL(head)) {
				TX_ADD_DIRECT(this);
				head = tail = node;
			} else {
				TX_ADD(tail);
				D_RW(tail)->next = node;

				TX_ADD_DIRECT(&tail);
				tail = node;
			}
		} TX_END;
	}

	int
	pop(PMEMobjpool* pop)
	{
		int value;
		TX_BEGIN(pop) {
			if (TOID_IS_NULL(head))
				pmemobj_tx_abort(-1);

			TOID(struct queue_node) head_ptr = head;
			value = D_RO(head)->value;

			TX_ADD_DIRECT(&head);
			head = D_RO(head)->next;
			TX_FREE(head_ptr);

			if (TOID_IS_NULL(head)) {
				TX_ADD_DIRECT(&tail);
				tail = TOID_NULL(struct queue_node);
            }
		} TX_END;

		return value;
	}

	void
	show()
	{
		TOID(struct queue_node) node = head;
		while (!TOID_IS_NULL(node)) {
			std::cout << "show: " << D_RO(node)->value << std::endl;
			node = D_RO(node)->next;
		}

		std::cout << std::endl;
	}

private:
	TOID(struct queue_node) head;
	TOID(struct queue_node) tail;
};

const char *ops_str[MAX_OPS] = {"push", "pop", "show"};

queue_op
parse_queue_ops(const std::string &ops)
{
	for (int i = 0; i < MAX_OPS; i++) {
		if (ops == ops_str[i]) {
			return (queue_op)i;
		}
	}
	return MAX_OPS;
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " pool" << std::endl;
		return 1;
	}

	auto path = argv[1];
	PMEMobjpool *pool = pmemobj_open(path, POBJ_LAYOUT_NAME(queue));
	if (pool == NULL)
		std::cerr << "failed to open the pool\n";

	TOID(struct queue) root = POBJ_ROOT(pool, struct queue);
	struct queue *q = D_RW(root);

	while (1) {
		std::cout << "[push value|pop|show]" << std::endl;

		std::string command;
		std::cin >> command;

		// parse string
		auto ops = parse_queue_ops(std::string(command));

		switch (ops) {
			case PUSH: {
				int value;
				std::cin >> value;

				q->push(pool, value);

				break;
			}
			case POP: {
				std::cout << q->pop(pool) << std::endl;
				break;
			}
			case SHOW: {
				q->show();
				break;
			}
			default: {
				std::cerr << "unknown ops" << std::endl;

				pmemobj_close(pool);
				exit(0);
			}
		}
	}
}
