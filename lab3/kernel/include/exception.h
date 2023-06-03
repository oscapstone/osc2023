#ifndef	_EXCEPTION_H_
#define	_EXCEPTION_H_

void invalid_exception_router(); // exception_handler.S
void el0_sync_router();
void el1h_irq_router();
#endif /*_EXCEPTION_H_*/