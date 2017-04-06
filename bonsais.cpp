#include <cstring>
#include <chrono>
#include <fstream>
#include <stack>

#include "BonsaiDCW.hpp"
#include "BonsaiPR.hpp"

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
    switch (type) {
      case Times::sec:
        return std::chrono::duration<double>(tp).count();
      case Times::milli:
        return std::chrono::duration<double, std::milli>(tp).count();
      case Times::micro:
        return std::chrono::duration<double, std::micro>(tp).count();
    }
    return 0.0;
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
    std::cerr << "ERROR: failed to open " << file_name << std::endl;
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
    std::cerr << "ERROR: failed to open " << file_name << std::endl;
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
int benchmark(const char* argv[]) {
  auto num_nodes = static_cast<uint64_t>(std::atoll(argv[4]));
  double load_factor = std::atof(argv[5]);
  auto colls_bits = static_cast<uint8_t>(std::atoi(argv[6]));

  // expecting that the concrete alphabet size is less than 253
  T bonsai{(uint64_t) (num_nodes / load_factor), 253, colls_bits};
  std::cout << "----- " << bonsai.name() << " -----" << std::endl;

  {
    KeyReader reader{argv[1]};
    if (!reader.is_ready()) {
      std::cerr << "ERROR: failed to open " << argv[1] << std::endl;
      return 1;
    }

    StopWatch sw;
    while (true) {
      auto key = reader.next();
      if (key.empty()) {
        break;
      }
      auto ptr = reinterpret_cast<const uint8_t*>(key.c_str());
      auto len = key.size() + 1; // including terminators
      bonsai.insert(ptr, len);
    }
    std::cout << "insert time: " << sw(Times::micro) / bonsai.num_strs() << " (ns/key)" << std::endl;
  }

  if (std::strcmp(argv[2], "-") != 0) {
    auto keys = read_keys(argv[2]);
    uint64_t ok = 0, ng = 0;
    StopWatch sw;
    for (const auto& key : keys) {
      auto ptr = reinterpret_cast<const uint8_t*>(key.c_str());
      auto len = key.size() + 1; // including terminators
      if (bonsai.search(ptr, len)) {
        ++ok;
      } else {
        ++ng;
      }
    }
    std::cout << "OK: " << ok << ", NG: " << ng << std::endl;
    std::cout << "search time: " << sw(Times::micro) / keys.size() << " (ns/key)" << std::endl;
  }

  bonsai.show_stat(std::cout);
  return 0;
}

}

int main(int argc, const char* argv[]) {
  std::ostringstream usage;
  usage << argv[0] << " <key> <query> <type> <#nodes> <load_factor> <colls_bits>";

  if (argc == 2) {
    std::cout << "#nodes: " << count_num_nodes(argv[1]) << std::endl;
    return 0;
  }

  if (argc == 7) {
    if (*argv[3] == '1') {
      return benchmark<BonsaiDCW>(argv);
    } else if (*argv[3] == '2') {
      return benchmark<BonsaiPR>(argv);
    }
  }

  std::cerr << usage.str() << std::endl;
  return 1;
}
