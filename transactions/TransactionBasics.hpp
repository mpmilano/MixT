#pragma once

struct TransactionContext {

	virtual bool commit() = 0;
};

