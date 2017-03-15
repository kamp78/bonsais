#include "BonsaiDCW.hpp"

namespace bonsais {

BonsaiDCW::BonsaiDCW(uint64_t num_slots, uint64_t alp_size, uint8_t colls_bits) {
  num_strs_ = 0;
  num_slots_ = num_slots;
  num_nodes_ = 1;
  alp_size_ = alp_size;
  colls_limit_ = 1U << colls_bits;

  root_id_ = {num_slots_ / 2, 0, num_slots_ / 2}; // without a particular reason
  empty_mark_ = alp_size * colls_limit_ + 2; // greater than the maximum quotient value expected

  prime_ = greater_prime(alp_size * colls_limit_ * num_slots_ + num_slots_ - 1);
  multiplier_ = UINT64_MAX / prime_; // Todo: support invertible

  if (num_bits(alp_size * colls_limit_ - 1) < num_bits(empty_mark_)) {
    std::cerr << "#bits required for alp_size * colls_limit < #bits allocated" << std::endl;
    std::cerr << "The former is " << (uint32_t) num_bits(alp_size * colls_limit_ - 1) << std::endl;
    std::cerr << "The latter is " << (uint32_t) num_bits(empty_mark_) << std::endl;
  }

  slots_ = sdsl::int_vector<>(num_slots, (empty_mark_ << 3) | (1U << 1),
                              num_bits(empty_mark_) + (uint8_t) 3);
  table_.fill(UINT8_MAX);

  set_quo_(root_id_.init_pos, 0); // other than empty_mark_
  set_vbit_(root_id_.init_pos, true);
}

bool BonsaiDCW::search(const uint8_t* str, uint64_t len) const {
  auto node_id = root_id_;
  for (uint64_t i = 0; i < len; ++i) {
    if (table_[str[i]] == UINT8_MAX) {
      return false;
    }
    if (!get_child_(node_id, static_cast<uint64_t>(table_[str[i]]))) {
      return false;
    }
  }
  return get_fbit_(node_id.slot_pos);
}

bool BonsaiDCW::insert(const uint8_t* str, uint64_t len) {
  auto node_id = root_id_;
  for (uint64_t i = 0; i < len; ++i) {
    if (table_[str[i]] == UINT8_MAX) {
      table_[str[i]] = alp_count_++;
      if (alp_size_ <= alp_count_) {
        std::cerr << "ERROR: alp_size_ < alp_count_" << std::endl;
        exit(1);
      }
    }
    add_child_(node_id, static_cast<uint64_t>(table_[str[i]]));
  }

  if (get_fbit_(node_id.slot_pos)) {
    return false;
  }

  set_fbit_(node_id.slot_pos, true);
  ++num_strs_;
  return true;
}

void BonsaiDCW::show_stat(std::ostream& os) const {
  os << "Bonsai stat." << std::endl;
  os << "num slots:   " << num_slots_ << std::endl;
  os << "num nodes:   " << num_nodes_ << std::endl;
  os << "load factor: " << static_cast<double>(num_nodes_) / num_slots_ << std::endl;
  os << "alp size:    " << alp_size_ << std::endl;
  os << "colls limit: " << colls_limit_ << std::endl;
  os << "size slots:  " << sdsl::size_in_bytes(slots_) << std::endl;
}

// expecting 0 <= quo <= alp_size + 1
HashValue BonsaiDCW::hash_(const NodeID& node_id, uint64_t symbol) const {
  uint64_t c = (symbol * colls_limit_ + node_id.num_colls) * num_slots_ + node_id.init_pos;
  uint64_t crnd = ((c % prime_) * multiplier_) % prime_; // avoiding overflow
  return {crnd % num_slots_, crnd / num_slots_};
}

bool BonsaiDCW::get_child_(NodeID& node_id, uint64_t symbol) const {
  if (alp_size_ <= symbol) {
    std::cerr << "ERROR: out-of-range symbol" << std::endl;
    exit(1);
  }

  const auto hv = hash_(node_id, symbol);
  if (empty_mark_ <= hv.quo) {
    std::cerr << "ERROR: out-of-range hv.quo" << std::endl;
    exit(1);
  }

  if (!get_vbit_(hv.rem)) {
    return false;
  }

  uint64_t dummy{};
  uint64_t pos = find_ass_cbit_pos_(hv.rem, dummy);
  if (pos == kNotFound) {
    return false;
  }

  uint64_t num_colls = find_item_(pos, hv.quo);
  if (colls_limit_ <= num_colls) {
    return false;
  }

  node_id = {hv.rem, num_colls, pos};
  return true;
}

bool BonsaiDCW::add_child_(NodeID& node_id, uint64_t symbol) {
  if (alp_size_ <= symbol) {
    std::cerr << "ERROR: out-of-range symbol" << std::endl;
    exit(1);
  }

  const auto hv = hash_(node_id, symbol);
  if (empty_mark_ <= hv.quo) {
    std::cerr << "ERROR: out-of-range hv.quo" << std::endl;
    exit(1);
  }

  if (get_quo_(hv.rem) == empty_mark_) {
    // without collision
    update_slot_(hv.rem, hv.quo, true, true, false);
    node_id = {hv.rem, 0, hv.rem};
    ++num_nodes_;
    return true;
  }

  uint64_t num_colls = 0, empty_pos = 0;
  uint64_t pos = find_ass_cbit_pos_(hv.rem, empty_pos);

  if (!get_vbit_(hv.rem)) { // initial insertion?
    // create a new collision group
    if (pos != kNotFound) { // require to displace existing groups?
      do {
        pos = right_(pos);
      } while (!get_cbit_(pos));

      pos = left_(pos); // rightmost slot of the group

      while (empty_pos != pos) {
        empty_pos = copy_from_right_(empty_pos);
      }
    } else {
      // not inside other collision groups
    }

    set_vbit_(hv.rem, true);
    set_cbit_(empty_pos, true);
  } else {
    // collision group already exists
    num_colls = find_item_(pos, hv.quo);

    if (num_colls < colls_limit_) { // already registered?
      node_id = {hv.rem, num_colls, pos};
      return false;
    }

    num_colls -= colls_limit_; // get original
    if (colls_limit_ <= num_colls) {
      std::cerr << "ERROR: exceeding #collisions" << std::endl;
      exit(1);
    }

    pos = left_(pos); // rightmost of the group

    // displace existing groups for creating an empty slot
    while (empty_pos != pos) {
      empty_pos = copy_from_right_(empty_pos);
    }
    set_cbit_(empty_pos, false);
  }

  set_quo_(empty_pos, hv.quo);
  set_fbit_(empty_pos, false);

  node_id = {hv.rem, num_colls, empty_pos};
  ++num_nodes_;

  return true;
}

// Finds the change bit associated with 'pos' and returns it.
// If not exist, returns kNotFound.
// Future, returns the rightmost empty slot located on the left side of 'pos'.
uint64_t BonsaiDCW::find_ass_cbit_pos_(uint64_t pos, uint64_t& empty_pos) const {
  assert(get_quo_(pos) != empty_mark_);

  // scan left slots until an empty slot is encountered,
  // with counting the number of valid virgin bits
  uint64_t num_vbits = 0;
  do {
    if (get_vbit_(pos)) {
      ++num_vbits;
    }
    pos = left_(pos);
  } while (get_quo_(pos) != empty_mark_);

  empty_pos = pos;

  if (num_vbits == 0) {
    return kNotFound;
  }

  // scan right slots until #cbits == #vbits
  uint64_t num_cbits = 0;
  while (num_cbits < num_vbits) {
    pos = right_(pos);
    if (get_cbit_(pos)) {
      ++num_cbits;
    }
  }

  return pos;
}

// Finds a proper slot in the collision group, and returns the slot pos and #collisions.
// If not exist, returns colls_limit_ + #slots in the group.
uint64_t BonsaiDCW::find_item_(uint64_t& pos, uint64_t quo) const {
  assert(get_cbit_(pos));

  uint64_t num_colls = 0;
  do {
    if (get_quo_(pos) == quo) {
      return num_colls;
    }
    pos = right_(pos);
    ++num_colls;
  } while (!get_cbit_(pos));

  return num_colls + colls_limit_;
}

uint64_t BonsaiDCW::right_(uint64_t pos) const {
  return pos == num_slots_ - 1 ? 0 : pos + 1;
}

uint64_t BonsaiDCW::left_(uint64_t pos) const {
  return pos == 0 ? num_slots_ - 1 : pos - 1;
}

// Copies a slot from the right slot except virgin bit information.
uint64_t BonsaiDCW::copy_from_right_(uint64_t pos) {
  auto _pos = right_(pos);
  slots_[pos] = (slots_[_pos] & vbit_inv_mask_) | (get_vbit_(pos) << 2);
  return _pos;
}

uint64_t BonsaiDCW::get_quo_(uint64_t pos) const {
  return slots_[pos] >> 3;
}

bool BonsaiDCW::get_vbit_(uint64_t pos) const {
  return ((slots_[pos] >> 2) & 1U) == 1U;
}

bool BonsaiDCW::get_cbit_(uint64_t pos) const {
  return ((slots_[pos] >> 1) & 1U) == 1U;
}

bool BonsaiDCW::get_fbit_(uint64_t pos) const {
  return (slots_[pos] & 1U) == 1U;
}

void BonsaiDCW::set_quo_(uint64_t pos, uint64_t quo) {
  slots_[pos] = (slots_[pos] & quo_inv_mask_) | (quo << 3);
}

void BonsaiDCW::set_vbit_(uint64_t pos, bool bit) {
  slots_[pos] = (slots_[pos] & vbit_inv_mask_) | (bit << 2);
}

void BonsaiDCW::set_cbit_(uint64_t pos, bool bit) {
  slots_[pos] = (slots_[pos] & cbit_inv_mask_) | (bit << 1);
}

void BonsaiDCW::set_fbit_(uint64_t pos, bool bit) {
  slots_[pos] = (slots_[pos] & fbit_inv_mask_) | bit;
}

void BonsaiDCW::update_slot_(uint64_t pos, uint64_t quo, bool vbit, bool cbit, bool fbit) {
  slots_[pos] = (quo << 3) | (vbit << 2) | (cbit << 1) | fbit;
}

} //bonsais
