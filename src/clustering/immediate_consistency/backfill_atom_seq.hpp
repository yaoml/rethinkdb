#ifndef CLUSTERING_IMMEDIATE_CONSISTENCY_HPP_
#define CLUSTERING_IMMEDIATE_CONSISTENCY_HPP_

#include "rdb_protocol/protocol.hpp"

/* A `backfill_atom_seq_t` contains all of the `backfill_{pre_}atom_t`s in some range of
the key-space. */

template<class atom_t>
class backfill_atom_seq_t {
public:
    /* Initializes an undefined seq that cannot be used for anything. */
    backfill_atom_seq_t() { }

    /* Initializes an empty seq with a zero-width region at the given location */
    backfill_atom_seq_t(
            uint64_t _beg_hash, uint64_t _end_hash, key_range_t::right_bound_t key) :
        beg_hash(_beg_hash), end_hash(_end_hash), left_key(key), right_key(key),
        mem_size(0) { }

    key_range_t::right_bound_t get_left_key() const { return left_key; }
    key_range_t::right_bound_t get_right_key() const { return right_key; }
    uint64_t get_beg_hash() const { return beg_hash; }
    uint64_t get_end_hash() const { return end_hash; }

    region_t get_region() const {
        if (left_key == right_key) {
            return region_t::empty();
        } else {
            key_range_t kr;
            kr.left = left_key.key;
            kr.right = right_key;
            return region_t(beg_hash, end_hash, kr);
        }
    }

    size_t get_mem_size() const { return mem_size; }

    typename std::list<atom_t>::const_iterator begin() const { return atoms.begin(); }
    typename std::list<atom_t>::const_iterator end() const { return atoms.end(); }
    bool empty() const { return atoms.empty(); }
    const atom_t &front() const { return atoms.front(); }

    /* Sets `*first_out` to the first atom whose left bound is to the left of
    `threshold`, or to `nullptr` if there is no such atom. The key-space beyond the end
    of the `backfill_atom_seq_t` is considered "unknown" space; if `threshold` is after
    `get_right_key()` and the seq is empty, we have no way of knowing what atom comes
    next. In this case we return `false`. */
    bool first_before_threshold(
            const key_range_t::right_bound_t &threshold,
            atom_t const **first_out) {
        if (atoms.empty() && right_key < threshold) {
            return false;
        }
        if (atoms.empty() || key_range_t::right_bound_t(
                front().get_range().left) >= threshold) {
            *first_out = nullptr;
        } else {
            *first_out = &front();
        }
        return true;
    }

    /* Deletes the leftmost atom in the seq. */
    void pop_front() {
        left_key = atoms.front().get_range().right;
        mem_size -= atoms.front().get_mem_size();
        atoms.pop_front();
    }

    /* Transfers the atom at the left end of this seq to the right end of the other seq.
    */
    void pop_front_into(backfill_atom_seq_t *other) {
        guarantee(beg_hash == other->beg_hash && end_hash == other->end_hash);
        guarantee(get_left_key() == other->get_right_key());
        size_t atom_size = atoms.front().get_mem_size();
        left_key = atoms.front().get_range().right;
        other->right_key = left_key;
        mem_size -= atom_size;
        other->mem_size += atom_size;
        other->atoms.splice(other->atoms.end(), atoms, atoms.begin());
    }

    /* Deletes the part of the seq that is to the left of the key. If a single backfill
    atom spans the key, that atom will be split. */
    void delete_to_key(const key_range_t::right_bound_t &cut) {
        guarantee(cut >= get_left_key());
        guarantee(cut <= get_right_key());
        while (!atoms.empty()) {
            key_range_t range = atoms.front().get_range();
            if (range.right <= cut) {
                mem_size -= atoms.front().get_mem_size();
                atoms.pop_front();
            } else if (key_range_t::right_bound_t(range.left) >= cut) {
                break;
            } else {
                range.left = cut.key;
                mem_size -= atoms.front().get_mem_size();
                atoms.front().mask_in_place(range);
                mem_size += atoms.front().get_mem_size();
                break;
            }
        }
        left_key = cut;
    }

    /* Appends an atom to the end of the seq. Atoms must be appended in lexicographical
    order. */
    void push_back(atom_t &&atom) {
        key_range_t atom_range = atom.get_range();
        guarantee(key_range_t::right_bound_t(atom_range.left) >= right_key);
        right_key = atom_range.right;
        mem_size += atom.get_mem_size();
        atoms.push_back(std::move(atom));
    }

    /* Indicates that there are no more atoms until the given key. */
    void push_back_nothing(const key_range_t::right_bound_t &bound) {
        guarantee(bound >= right_key);
        right_key = bound;
    }

    /* Concatenates two `backfill_atom_seq_t`s. They must be adjacent. */
    void concat(backfill_atom_seq_t &&other) {
        guarantee(beg_hash == other.beg_hash && end_hash == other.end_hash);
        guarantee(right_key == other.left_key);
        right_key = other.right_key;
        mem_size += other.mem_size;
        atoms.splice(atoms.end(), std::move(other.atoms));
    }

private:
    /* A `backfill_atom_seq_t` has a `region_t`, with one oddity: even when the region
    is empty, the `backfill_atom_seq_t` still has a meaningful left and right bound. This
    is why this is stored as four separate variables instead of a `region_t`. We use two
    `key_range_t::right_bound_t`s instead of a `key_range_t` so that we can represent
    a zero-width region after the last key. */
    uint64_t beg_hash, end_hash;
    key_range_t::right_bound_t left_key, right_key;

    /* The cumulative byte size of the atoms (i.e. the sum of `a.get_mem_size()` over all
    the atoms) */
    size_t mem_size;

    std::list<atom_t> atoms;

    RDB_MAKE_ME_SERIALIZABLE_6(backfill_atom_seq_t,
        beg_hash, end_hash, left_key, right_key, mem_size, atoms);
};

#endif /* CLUSTERING_IMMEDIATE_CONSISTENCY_HPP_ */
