#include "BonsaiPR.hpp"

namespace bonsais {

BonsaiPR::BonsaiPR(uint64_t num_slots, uint64_t alp_size, uint8_t width_1st) {
  num_strs_ = 0;

  num_slots_ = num_slots;
  num_nodes_ = 1;
  alp_size_ = alp_size;
  width_1st_ = width_1st;

  root_id_ = num_slots / 2; // without a particular reason
  empty_mark_ = alp_size + 2; // greater than the maximum quotient value expected
  max_dsp1st_ = (1U << width_1st) - 1;

  prime_ = greater_prime(alp_size * num_slots + num_slots - 1);
  multiplier_ = UINT64_MAX / prime_; // Todo: support invertible

  if (num_bits(alp_size - 1) < num_bits(empty_mark_)) {
    std::cerr << "Note that #bits required for alp_size < #bits allocated" << std::endl;
    std::cerr << "The former is " << (uint32_t) num_bits(alp_size - 1) << std::endl;
    std::cerr << "The latter is " << (uint32_t) num_bits(empty_mark_) << std::endl;
  }

  FitVector(num_slots, num_bits(empty_mark_) + width_1st + 1U,
            empty_mark_ << (width_1st + 1U)).swap(slots_);
  table_.fill(UINT8_MAX);
}

bool BonsaiPR::search(const uint8_t* str, uint64_t len) const {
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

bool BonsaiPR::insert(const uint8_t* str, uint64_t len) {
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

void BonsaiPR::show_stat(std::ostream& os) const {
  os << "BonsaiPlus stat." << std::endl;
  os << "num slots:   " << num_slots_ << std::endl;
  os << "num nodes:   " << num_nodes_ << std::endl;
  os << "load factor: " << static_cast<double>(num_nodes_) / num_slots_ << std::endl;
  os << "num auxs:    " << aux_map_.size() << std::endl;
  os << "auxs rate:   " << static_cast<double>(aux_map_.size()) / num_slots_ << std::endl;
  os << "alp size:    " << alp_size_ << std::endl;
  os << "width 1st:   " << (uint32_t) width_1st_ << std::endl;
  os << "size slots:  " << slots_.size_in_bytes() << std::endl;
  os << "average dsp: " << calc_ave_dsp() << std::endl;
}

double BonsaiPR::calc_ave_dsp() const {
  uint64_t num_used_slots = 0, sum_dsp = 0;
  for (uint64_t i = 0; i < num_slots_; ++i) {
    if (get_quo_(i) != empty_mark_) {
      ++num_used_slots;
      sum_dsp += get_dsp_(i);
    }
  }
  return double(sum_dsp) / num_used_slots;
}

// expecting 0 <= quo <= alp_size + 1
HashValue BonsaiPR::hash_(uint64_t node_id, uint64_t symbol) const {
  uint64_t c = symbol * num_slots_ + node_id;
  uint64_t c_rnd = ((c % prime_) * multiplier_) % prime_; // avoiding overflow
  return {c_rnd % num_slots_, c_rnd / num_slots_};
}

bool BonsaiPR::get_child_(uint64_t& node_id, uint64_t symbol) const {
  if (alp_size_ <= symbol) {
    std::cerr << "ERROR: out-of-range symbol" << std::endl;
    exit(1);
  }

  const auto hv = hash_(node_id, symbol);
  if (empty_mark_ <= hv.quo) {
    std::cerr << "ERROR: out-of-range hv.quo" << std::endl;
    exit(1);
  }

  for (uint64_t pos = hv.rem, cnt = 0;; pos = right_(pos), ++cnt) {
    if (pos == root_id_) {
      continue;
    }

    const uint64_t quo = get_quo_(pos);

    if (quo == empty_mark_) {
      return false;
    }

    if (quo == hv.quo && get_dsp_(pos) == cnt) { // already registered?
      node_id = pos;
      return true;
    }
  }
}

bool BonsaiPR::add_child_(uint64_t& node_id, uint64_t symbol, bool is_tail) {
  if (alp_size_ <= symbol) {
    std::cerr << "ERROR: out-of-range symbol" << std::endl;
    exit(1);
  }

  const auto hv = hash_(node_id, symbol);
  if (empty_mark_ <= hv.quo) {
    std::cerr << "ERROR: out-of-range hv.quo" << std::endl;
    exit(1);
  }

  for (uint64_t pos = hv.rem, cnt = 0;; pos = right_(pos), ++cnt) {
    if (pos == root_id_) {
      continue;
    }

    const uint64_t quo = get_quo_(pos);

    if (quo == empty_mark_) {
      update_slot_(pos, hv.quo, cnt, false);
      node_id = pos;
      ++num_nodes_;
      return true;
    }

    if (is_tail) {
      continue;
    }

    if (quo == hv.quo && get_dsp_(pos) == cnt) { // already registered?
      node_id = pos;
      return false;
    }
  }
}

uint64_t BonsaiPR::right_(uint64_t pos) const {
  return ++pos >= num_slots_ ? 0 : pos;
}

uint64_t BonsaiPR::get_quo_(uint64_t pos) const {
  return slots_.get(pos) >> (width_1st_ + 1);
}

uint64_t BonsaiPR::get_dsp_(uint64_t pos) const {
  uint64_t dsp = (slots_.get(pos) >> 1) & max_dsp1st_;
  if (dsp < max_dsp1st_) {
    return dsp;
  }
  auto it = aux_map_.find(pos);
  return it == aux_map_.end() ? kNotFound : it->second;
}

bool BonsaiPR::get_fbit_(uint64_t pos) const {
  return (slots_.get(pos) & 1U) == 1U;
}

void BonsaiPR::set_fbit_(uint64_t pos, bool bit) {
  slots_.set(pos, (slots_.get(pos) & ~1U) | bit);
}

void BonsaiPR::update_slot_(uint64_t pos, uint64_t quo, uint64_t dsp, bool fbit) {
  uint64_t val = quo << (width_1st_ + 1);

  if (dsp < max_dsp1st_) {
    val |= (dsp << 1);
  } else {
    val |= (max_dsp1st_ << 1);
    assert(aux_map_.find(pos) == aux_map_.end());
    aux_map_.insert({pos, dsp});
  }

  slots_.set(pos, val | fbit);
}

} // bonsais