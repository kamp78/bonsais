#ifndef BONSAIS_BONSAI_PLUS_HPP
#define BONSAIS_BONSAI_PLUS_HPP

#include "Basics.hpp"

namespace bonsais {

/*
 * Very simple implementation of m-Bonsai (recursive) described in
 * - Poyias and Raman, Improved practical compact dynamic tries, SPIRE, 2015.
 * */
class BonsaiPR {
public:
  BonsaiPR(uint64_t num_slots, uint64_t alp_size, uint8_t width_1st);
  ~BonsaiPR() {}

  static std::string name() { return "BonsaiPR"; }

  bool search(const uint8_t* str, uint64_t len) const {
    uint64_t node_id = root_id_;
    for (uint64_t i = 0; i < len; ++i) {
      if (table_[str[i]] == UINT8_MAX) {
        return false;
      }
      if (!get_child_(node_id, static_cast<uint64_t>(table_[str[i]]))) {
        return false;
      }
    }
    return get_fbit_(node_id);
  }

  template<typename T>
  bool search(const T* str, uint64_t len) const {
    static_assert(Is_pod<T>(), "T is not POD.");

    uint64_t node_id = root_id_;
    for (uint64_t i = 0; i < len; ++i) {
      if (!get_child_(node_id, static_cast<uint64_t>(str[i]))) {
        return false;
      }
    }
    return get_fbit_(node_id);
  }

  bool insert(const uint8_t* str, uint64_t len) {
    uint64_t node_id = root_id_;
    bool is_tail = false;
    for (uint64_t i = 0; i < len; ++i) {
      if (table_[str[i]] == UINT8_MAX) {
        table_[str[i]] = alp_count_++;
        if (alp_size_ <= alp_count_) {
          std::cerr << "ERROR: alp_size_ < alp_count_" << std::endl;
          exit(1);
        }
      }
      is_tail = add_child_(node_id, static_cast<uint64_t>(table_[str[i]]), is_tail);
    }
    if (get_fbit_(node_id)) {
      assert(!is_tail);
      return false;
    }
    set_fbit_(node_id, true);
    ++num_strs_;
    return true;
  }

  template<typename T>
  bool insert(const T* str, uint64_t len) {
    static_assert(Is_pod<T>(), "T is not POD.");

    uint64_t node_id = root_id_;
    bool is_tail = false;
    for (uint64_t i = 0; i < len; ++i) {
      is_tail = add_child_(node_id, static_cast<uint64_t>(str[i]), is_tail);
    }
    if (get_fbit_(node_id)) {
      assert(!is_tail);
      return false;
    }
    set_fbit_(node_id, true);
    ++num_strs_;
    return true;
  }

  uint64_t num_strs() const { return num_strs_; }
  void show_stat(std::ostream& os) const;

  double calc_ave_dsp() const;

  BonsaiPR(const BonsaiPR&) = delete;
  BonsaiPR& operator=(const BonsaiPR&) = delete;

private:
  uint64_t num_strs_;
  uint64_t num_slots_;
  uint64_t num_nodes_;
  uint64_t alp_size_;
  uint8_t width_1st_;

  uint64_t root_id_;
  uint64_t empty_mark_;
  uint64_t max_dsp1st_; // maximum displacement value in 1st layer

  uint64_t prime_;
  uint64_t multiplier_;

  sdsl::int_vector<> slots_; // with quotient value, displacement value, and final bit
  std::map<uint64_t, uint32_t> aux_map_; // for exceeding displacement values

  // used for strings composed of uint8_t
  std::array<uint8_t, 256> table_;
  uint8_t alp_count_ = 0;

  HashValue hash_(uint64_t node_id, uint64_t symbol) const;

  bool get_child_(uint64_t& node_id, uint64_t symbol) const;
  bool add_child_(uint64_t& node_id, uint64_t symbol, bool is_tail = false);

  uint64_t right_(uint64_t pos) const;

  uint64_t get_quo_(uint64_t pos) const;
  uint64_t get_dsp_(uint64_t pos) const;
  bool get_fbit_(uint64_t pos) const;
  void set_fbit_(uint64_t pos, bool bit);

  void update_slot_(uint64_t pos, uint64_t quo, uint64_t dsp, bool fbit);
};

} //bonsais

#endif //BONSAIS_BONSAI_PLUS_HPP
