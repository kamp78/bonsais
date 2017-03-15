#ifndef BONSAIS_BONSAI_HPP
#define BONSAIS_BONSAI_HPP

#include "Basics.hpp"

namespace bonsais {

/*
 * Bonsai structure described in
 * - Darragh, Cleary and Witten, Bonsai: A compact representation of trees, SPE, 1993.
 * */
class BonsaiDCW {
public:
  BonsaiDCW(uint64_t num_slots, uint64_t alp_size, uint8_t colls_bits);
  ~BonsaiDCW() {}

  static std::string name() { return "BonsaiDCW"; }

  bool search(const uint8_t* str, uint64_t len) const;
  template<typename T> bool search(const T* str, uint64_t len) const;

  bool insert(const uint8_t* str, uint64_t len);
  template<typename T> bool insert(const T* str, uint64_t len);

  uint64_t num_strs() const { return num_strs_; }
  void show_stat(std::ostream& os) const;

  BonsaiDCW(const BonsaiDCW&) = delete;
  BonsaiDCW& operator=(const BonsaiDCW&) = delete;

private:
  struct NodeID {
    uint64_t init_pos;
    uint64_t num_colls;
    uint64_t slot_pos; // for convenience
  };

  uint64_t num_strs_;
  uint64_t num_slots_;
  uint64_t num_nodes_;
  uint64_t alp_size_;
  uint32_t colls_limit_;

  NodeID root_id_;
  uint64_t empty_mark_;

  uint64_t prime_;
  uint64_t multiplier_;

  sdsl::int_vector<> slots_; // with quotient value, virgin bit, change bit, and final bit

  const uint64_t quo_inv_mask_ = 7U;
  const uint64_t vbit_inv_mask_ = ~(1U << 2);
  const uint64_t cbit_inv_mask_ = ~(1U << 1);
  const uint64_t fbit_inv_mask_ = ~1U;

  // used for strings composed of uint8_t
  std::array<uint8_t, 256> table_;
  uint8_t alp_count_ = 0;

  HashValue hash_(const NodeID& node_id, uint64_t symbol) const;

  bool get_child_(NodeID& node_id, uint64_t symbol) const;
  bool add_child_(NodeID& node_id, uint64_t symbol);

  uint64_t find_ass_cbit_pos_(uint64_t pos, uint64_t& empty_pos) const;
  uint64_t find_item_(uint64_t& pos, uint64_t quo) const;

  uint64_t right_(uint64_t pos) const;
  uint64_t left_(uint64_t pos) const;
  uint64_t copy_from_right_(uint64_t pos);

  uint64_t get_quo_(uint64_t pos) const;
  bool get_vbit_(uint64_t pos) const;
  bool get_cbit_(uint64_t pos) const;
  bool get_fbit_(uint64_t pos) const;

  void set_quo_(uint64_t pos, uint64_t quo);
  void set_vbit_(uint64_t pos, bool bit);
  void set_cbit_(uint64_t pos, bool bit);
  void set_fbit_(uint64_t pos, bool bit);

  void update_slot_(uint64_t pos, uint64_t quo, bool vbit, bool cbit, bool fbit);
};

template<typename T>
bool BonsaiDCW::search(const T* str, uint64_t len) const {
  static_assert(Is_pod<T>(), "T is not POD.");

  auto node_id = root_id_;
  for (uint64_t i = 0; i < len; ++i) {
    if (!get_child_(node_id, static_cast<uint64_t>(str[i]))) {
      return false;
    }
  }
  return get_fbit_(node_id.slot_pos);
}

template<typename T>
bool BonsaiDCW::insert(const T* str, uint64_t len) {
  static_assert(Is_pod<T>(), "T is not POD.");

  auto node_id = root_id_;
  for (uint64_t i = 0; i < len; ++i) {
    add_child_(node_id, static_cast<uint64_t>(str[i]));
  }

  if (get_fbit_(node_id.slot_pos)) {
    return false;
  }

  set_fbit_(node_id.slot_pos, true);
  ++num_strs_;
  return true;
}

} //bonsais

#endif //BONSAIS_BONSAI_HPP
