#include <include/syscall_helpers.hpp>
#include <include/native_heap.hpp>
using namespace riscv;
static const uint64_t ARENA_BASE = 0x40000000;
static sas_alloc::Arena arena;

static const uint32_t SYSCALL_MALLOC  = 1;
static const uint32_t SYSCALL_CALLOC  = 2;
//static const uint32_t SYSCALL_REALLOC = 3;
static const uint32_t SYSCALL_FREE    = 4;

template <int W>
static long syscall_malloc(Machine<W>& machine)
{
	const size_t len = machine.template sysarg<address_type<W>>(0);
	auto data = arena.malloc(len);
	SYSPRINT("SYSCALL malloc(%zu) = 0x%X\n", len, data);
	return data;
}
template <int W>
static long syscall_calloc(Machine<W>& machine)
{
	const size_t count = machine.template sysarg<address_type<W>>(0);
	const size_t size  = machine.template sysarg<address_type<W>>(1);
	const size_t len = count * size;
	auto data = arena.malloc(len);
	SYSPRINT("SYSCALL calloc(%zu, %zu) = 0x%X\n", count, size, data);
	if (data != 0) {
		// TODO: optimize this (CoW), **can throw**
		machine.memory.memset(data, 0, len);
	}
	return data;
}
template <int W>
static long syscall_free(Machine<W>& machine)
{
	const auto ptr = machine.template sysarg<address_type<W>>(0);
	int ret = arena.free(ptr);
	SYSPRINT("SYSCALL free(0x%X) = %d\n", ptr, ret);
	return ret; /* avoid returning something here? */
}


template <int W>
void setup_native_heap_syscalls(Machine<W>& machine, size_t max_memory)
{
	machine.install_syscall_handler(SYSCALL_MALLOC, syscall_malloc<W>);
	machine.install_syscall_handler(SYSCALL_CALLOC, syscall_calloc<W>);
	machine.install_syscall_handler(SYSCALL_FREE,   syscall_free<W>);
	arena = sas_alloc::Arena(ARENA_BASE, ARENA_BASE + max_memory);
}

/* le sigh */
template void setup_native_heap_syscalls<4>(Machine<4>&, size_t);
