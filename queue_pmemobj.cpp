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

enum queue_op {
	PUSH,
	POP,
	SHOW,
	EXIT,
	MAX_OPS,
};

struct queue_node {
	int value;
	PMEMoid next;
};

struct queue {
	void
	push(PMEMobjpool *pop, int value)
	{
		TX_BEGIN(pop) {
			PMEMoid node = pmemobj_tx_alloc(sizeof(struct queue_node), 0);
			((struct queue_node*) pmemobj_direct(node))->value = value;
			((struct queue_node*) pmemobj_direct(node))->next = OID_NULL;

			if (OID_IS_NULL(head)) {
				pmemobj_tx_add_range_direct(this, sizeof(*this));
				head = tail = node;
			} else {
				pmemobj_tx_add_range(tail, 0, sizeof(struct queue_node));
				((struct queue_node*) pmemobj_direct(tail))->next = node;

				pmemobj_tx_add_range_direct(&tail, sizeof(tail));
				tail = node;
			}
		} TX_END;
	}

	int
	pop(PMEMobjpool* pop)
	{
		int value;
		TX_BEGIN(pop) {
			if (OID_IS_NULL(head))
				pmemobj_tx_abort(-1);

			PMEMoid head_ptr = head;
			value = ((struct queue_node*) pmemobj_direct(head))->value;

			pmemobj_tx_add_range_direct(&head, sizeof(head));
			head = ((struct queue_node*) pmemobj_direct(head))->next;
			pmemobj_tx_free(head_ptr);

			if (OID_IS_NULL(head)) {
				pmemobj_tx_add_range_direct(&tail, sizeof(tail));
				tail = OID_NULL;
			}
		} TX_END;

		return value;
	}

	void
	show()
	{
		PMEMoid node = head;
		while (!OID_IS_NULL(node)) {
			struct queue_node *node_p = (struct queue_node*)pmemobj_direct(node);

			std::cout << "show: " << node_p->value << std::endl;
			node = node_p->next;
		}

		std::cout << std::endl;
	}

private:
	PMEMoid head;
	PMEMoid tail;
};

const char *ops_str[MAX_OPS] = {"push", "pop", "show", "exit"};

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
	PMEMobjpool *pool = pmemobj_open(path, "queue");
	if (pool == NULL)
		std::cerr << "failed to open the pool\n";

	PMEMoid root = pmemobj_root(pool, sizeof(struct queue));
	struct queue *q = (struct queue*) pmemobj_direct(root);

	while (1) {
		std::cout << "[push value|pop|show|exit]" << std::endl;

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
			case EXIT: {
				pmemobj_close(pool);
				exit(0);
			}
			default: {
				std::cerr << "unknown ops" << std::endl;
				pmemobj_close(pool);
				exit(0);
			}
		}
	}
}
