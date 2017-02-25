#include "Bonsai.hpp"
#include "BonsaiPlus.hpp"

using namespace bonsais;

namespace {

enum class Times {
  sec, milli, micro
};

class StopWatch {
public:
  StopWatch() : tp_(std::chrono::high_resolution_clock::now()) {}
  ~StopWatch() {}

  double operator()(Times type) const {
    auto tp = std::chrono::high_resolution_clock::now() - tp_;
    double ret;

    switch (type) {
      case Times::sec:
        ret = std::chrono::duration<double>(tp).count();
        break;
      case Times::milli:
        ret = std::chrono::duration<double, std::milli>(tp).count();
        break;
      case Times::micro:
        ret = std::chrono::duration<double, std::micro>(tp).count();
        break;
    }

    return ret;
  }

  StopWatch(const StopWatch&) = delete;
  StopWatch& operator=(const StopWatch&) = delete;

private:
  std::chrono::high_resolution_clock::time_point tp_;
};

class KeyReader {
public:
  KeyReader(const char* file_name) {
    std::ios::sync_with_stdio(false);
    buf_.reserve(1 << 10);
    ifs_.open(file_name);
  }
  ~KeyReader() {}

  bool is_ready() const {
    return !ifs_.fail();
  }

  const std::string& next() {
    std::getline(ifs_, buf_);
    return buf_;
  }

  KeyReader(const KeyReader&) = delete;
  KeyReader& operator=(const KeyReader&) = delete;

private:
  std::ifstream ifs_;
  std::string buf_;
};

std::vector<std::string> read_keys(const char* file_name) {
  std::ifstream ifs{file_name};
  if (!ifs) {
    std::cerr << "error : failed to open " << file_name << std::endl;
    return {};
  }

  std::string line;
  std::vector<std::string> keys;

  while (std::getline(ifs, line)) {
    if (line.empty()) {
      continue;
    }
    keys.push_back(line);
  }

  return keys;
}

uint64_t count_num_nodes(const char* file_name) {
  auto keys = read_keys(file_name);
  if (keys.empty()) {
    std::cerr << "error : failed to open " << file_name << std::endl;
    return 0;
  }

  std::sort(keys.begin(), keys.end());

  struct Node {
    size_t begin;
    size_t end;
    size_t depth;
  };

  std::stack<Node> nodes;
  nodes.push({0,keys.size(),0});

  uint64_t num_nodes = 1;

  while (!nodes.empty()) {
    Node node = nodes.top();
    nodes.pop();

    while (node.begin < node.end && keys[node.begin].length() == node.depth) {
      ++node.begin;
    }
    if (node.begin == node.end) {
      continue;
    }

    for (auto i = node.begin + 1; i < node.end; ++i) {
      if (keys[i - 1][node.depth] != keys[i][node.depth]) {
        nodes.push({node.begin, i, node.depth + 1});
        node.begin = i;
        ++num_nodes;
      }
    }
    nodes.push({node.begin, node.end, node.depth + 1});
    ++num_nodes;
  }

  return num_nodes + keys.size(); // including terminators
}

template<typename T>
bool run_insertion(T& bonsai, KeyReader& reader) {
  size_t num_keys = 0;
  StopWatch sw;

  while (true) {
    auto key = reader.next();
    if (key.empty()) {
      break;
    }

    auto ptr = reinterpret_cast<const uint8_t*>(key.c_str());
    auto len = key.size() + 1; // including terminators
    if (!bonsai.insert(ptr, len)) {
      std::cerr << "failed to insert " << key << std::endl;
      return false;
    }

    ++num_keys;
  }

  std::cout << "insertion time : " << sw(Times::micro) / num_keys << " (ns / key)" << std::endl;
  return true;
}

template<typename T>
bool run_search(const T& bonsai, const std::vector<std::string>& keys) {
  StopWatch sw;

  for (const auto& key : keys) {
    auto ptr = reinterpret_cast<const uint8_t*>(key.c_str());
    auto len = key.size() + 1; // including terminators
    if (!bonsai.search(ptr, len)) {
      std::cerr << "failed to search " << key << std::endl;
      return false;
    }
  }

  std::cout << "search time : " << sw(Times::micro) / keys.size() << " (ns / key)" << std::endl;
  return true;
}

template<typename T>
int benchmark(const char* argv[]) {
  auto num_nodes = static_cast<uint64_t>(std::atoll(argv[4]));
  double load_factor = std::atof(argv[5]);
  auto colls_bits = static_cast<uint8_t>(std::atoi(argv[6]));

  // expecting that ASCII codes 253, 254 and 255 are unused
  T bonsai{(uint64_t) (num_nodes / load_factor), 253, colls_bits};
  std::cout << "benchmark of " << bonsai.name() << std::endl;

  {
    KeyReader reader{argv[1]};
    if (!reader.is_ready()) {
      std::cerr << "error : failed to open " << argv[1] << std::endl;
      return 1;
    }
    if (!run_insertion(bonsai, reader)) {
      return 1;
    }
  }

  if (std::strcmp(argv[2], "-") != 0) {
    auto keys = read_keys(argv[2]);
    if (!run_search(bonsai, keys)) {
      return 1;
    }
  }

  bonsai.show_stat(std::cout);
  return 0;
}

}

int main(int argc, const char* argv[]) {
  std::ostringstream usage;
  usage << argv[0] << " <key> <query> <type> <#nodes> <load_factor> <colls_bits>";

  if (argc == 2) {
    std::cout << "#nodes : " << count_num_nodes(argv[1]) << std::endl;
    return 0;
  }

  if (argc == 7) {
    if (*argv[3] == '1') {
      return benchmark<Bonsai>(argv);
    } else if (*argv[3] == '2') {
      return benchmark<BonsaiPlus>(argv);
    }
  }

  std::cerr << usage.str() << std::endl;
  return 1;
}
