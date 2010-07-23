
#ifndef __COREFWD_HPP__
#define __COREFWD_HPP__

// FSM
template<class config_t>
struct conn_fsm;

// Serializers
template <class config_t>
struct in_place_serializer_t;

// Caches

template <class config_t>
struct array_map_t;

template <class config_t>
struct page_repl_random_t;

template <class config_t>
struct mirrored_cache_t;

template <class config_t>
struct buf;

template <class config_t>
struct transaction;

template <class config_t>
struct writeback_tmpl_t;

template<class config_t>
struct rwi_conc_t;

// Btree
struct btree_node;

template <typename config_t>
class btree_fsm;

template <typename config_t>
class btree_get_fsm;

template <typename config_t>
class btree_set_fsm;

template <typename config_t>
class btree_delete_fsm;

template <class config_t>
class btree_admin;

// Event queue
struct event_t;
struct event_queue_t;

// Worker pool
struct worker_pool_t;

// Request handler
template <class config_t>
class request_handler_t;

template <class config_t>
class txt_memcached_handler_t;

template <class config_t>
class bin_memcached_handler_t;

template <class config_t>
class memcached_handler_t;

// Context
struct iocb;

template <class config_t>
class request;

// IO Calls
struct posix_io_calls_t;

// Allocators
template <class super_alloc_t>
struct dynamic_pool_alloc_t;

template <class super_alloc_t>
struct pool_alloc_t;

template <class super_alloc_t>
struct alloc_stats_t;

template <class accessor_t, class type_t>
class alloc_mixin_t;

template <class alloc_t>
class tls_small_obj_alloc_accessor;

// Utils
template <int _size>
struct buffer_t;

// Containers
template <class derived_t>
class intrusive_list_node_t;

template <class node_t>
class intrusive_list_t;

#endif // __COREFWD_HPP__

