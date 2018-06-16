#include <future>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#include "label.h"
#include "phrasalrule.h"
#include "sentence.h"
#include "tree.h"

namespace {

std::mutex inputLock, outputLock;

bool process() {
  std::string line;
  {
    std::lock_guard g(inputLock);
    if (!std::getline(std::cin, line)) {
      return false;
    }
  }
  try {
    auto asp = jhu::thrax::readAlignedSentencePair<false, true>(line);
    auto tree = jhu::thrax::readTree(jhu::thrax::fields(line)[1]);
    jhu::thrax::SAMTLabeler samt{std::move(tree)};
    std::ostringstream out;
    for (const auto& rule : jhu::thrax::extract(asp, 10)) {
      out << jhu::thrax::LabeledRuleView{ rule, samt } << '\n';
    }
    std::lock_guard g(outputLock);
    std::cout << out.str();
  } catch (std::exception& e) {
    std::cerr << e.what() << ' ' << line << '\n';
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  int threads = 1;
  if (argc > 1) {
    threads = std::atoi(argv[1]);
  }
  std::ios::sync_with_stdio(false);
  if (threads < 2) {
    while (process()) {}
    return 0;
  }
  std::vector<std::future<void>> workers;
  workers.reserve(threads);
  for (int i = 0; i < threads; i++) {
    workers.push_back(std::async([]() { while (process()) {} }));
  }
  for (auto& f : workers) {
    f.get();
  }
}