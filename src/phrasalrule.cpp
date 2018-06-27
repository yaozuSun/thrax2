#include "phrasalrule.h"

#include <optional>

namespace jhu::thrax {

namespace {

std::optional<PhrasalRule> addNonterminal(const PhrasalRule& r, SpanPair nt) {
  if (!r.lhs.contains(nt)) {
    return {};
  }
  auto it = r.nts.begin() + r.nextNT;
  if (it == r.nts.end()) {
    return {};
  }
  if (it > r.nts.begin() && nt.src.start < (it - 1)->src.start) {
    return {};
  }
  if (!std::all_of(
        r.nts.begin(), it, [nt](auto i) {
          return disjoint(i, nt);
        })) {
    return {};
  }
  PhrasalRule result(r);
  result.nts[result.nextNT++] = nt;
  cutPoints(result.alignment, nt.src.start, nt.src.end);
  return result;
}

using Rules = std::vector<PhrasalRule>;

Rules addAllNonterminals(
    const Rules& rules, const std::vector<SpanPair>& phrases) {
  Rules next;
  next.reserve(rules.size());
  for (const auto& rule : rules) {
    auto it = std::lower_bound(
        phrases.begin(), phrases.end(), rule.lhs, bySourceStart);
    for (; it < phrases.end(); ++it) {
      if (auto r = addNonterminal(rule, *it); r.has_value()) {
        next.push_back(*std::move(r));
      } else if (it->src.start >= rule.lhs.src.end) {
        break;
      }
    }
  }
  return next;
}

template<typename T, size_t N>
std::vector<T> cat(std::array<std::vector<T>, N>&& vals) {
  auto sz = std::accumulate(
      vals.begin(),
      vals.end(),
      size_t{0},
      [](size_t total, const auto& v) { return total + v.size(); });
  std::vector<T> result;
  result.reserve(sz);
  for (auto& v : vals) {
    std::move(v.begin(), v.end(), std::back_inserter(result));
  }
  return result;
}

}

std::vector<PhrasalRule> extract(
    const AlignedSentencePair& sentence, std::vector<SpanPair> initial) {
  std::sort(initial.begin(), initial.end(), bySourceStart);
  std::array<Rules, kMaxNonterminals + 1> rules;
  rules.front().reserve(initial.size());
  for (auto p : initial) {
    rules.front().emplace_back(sentence, p);
  }
  for (size_t i = 0; i < kMaxNonterminals; i++) {
    rules[i + 1] = addAllNonterminals(rules[i], initial);
  }
  return cat(std::move(rules));
}

}
