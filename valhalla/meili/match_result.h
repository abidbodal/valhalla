// -*- mode: c++ -*-
#ifndef MMP_MATCH_RESULT_H_
#define MMP_MATCH_RESULT_H_

#include <algorithm>
#include <vector>

#include <valhalla/baldr/graphid.h>
#include <valhalla/baldr/graphreader.h>
#include <valhalla/meili/stateid.h>
#include <valhalla/midgard/pointll.h>

namespace valhalla {
namespace meili {

struct MatchResult {
  // Coordinate of the match point
  midgard::PointLL lnglat;
  // Distance from measurement to the match point
  float distance_from;
  // Which edge this match point stays
  baldr::GraphId edgeid;
  // Percentage distance along the edge
  float distance_along;
  // Optional epoch time for this match point copied from original measurement
  double epoch_time;
  // Sequential state id
  StateId stateid;

  bool HasState() const {
    return stateid.IsValid();
  }
};

struct EdgeSegment {
  EdgeSegment(baldr::GraphId edgeid,
              float source,
              float target,
              std::vector<MatchResult>::const_iterator firstMatch,
              std::vector<MatchResult>::const_iterator lastMatch);

  std::vector<midgard::PointLL> Shape(baldr::GraphReader& graph_reader) const;
  bool Adjoined(baldr::GraphReader& graph_reader, const EdgeSegment& other) const;

  std::vector<MatchResult>::const_iterator firstMatch;
  std::vector<MatchResult>::const_iterator lastMatch;
  baldr::GraphId edgeid{baldr::kInvalidGraphId};
  float source{0.f};
  float target{1.f};
  bool discontinuity{false};
};

struct MatchResults {
  MatchResults(std::vector<MatchResult>&& results, std::vector<EdgeSegment>&& segments, float score)
      : results(results), segments(segments), score(score) {
    edges.reserve(this->segments.size());
    for (const auto& segment : this->segments)
      if (edges.empty() || edges.back() != segment.edgeid)
        edges.push_back(segment.edgeid);
    e1 = this->segments.empty() || this->segments.front().source < 1.0f ? edges.cbegin()
                                                                        : edges.cbegin() + 1;
    e2 = this->segments.empty() || this->segments.back().target > 0.0f ? edges.cend()
                                                                       : edges.cend() - 1;
  }
  bool operator==(const MatchResults& p) const {
    // find the beginning of that path in this one
    auto f = std::find(e1, e2, *p.e1);
    // maybe this path starts inside of p
    if (f == e2) {
      f = std::find(p.e1, p.e2, *e1);
      // if this path started inside of p, and whats left of p is smaller than this path, search for
      // it in this path if whats left of this path is larger than p, search for p within this
      // larger
      return f != p.e2 && p.e2 - f < e1 - e2 ? std::equal(f, p.e2, e1) : std::equal(e1, e2, f);
    }
    // p started inside of this path, if whats left of this path is smaller than p, search for it in
    // p if whats left of this path is larger than p, search for p within this larger
    return e2 - f < p.e1 - p.e2 ? std::equal(f, e2, p.e1) : std::equal(p.e1, p.e2, f);
  }

  MatchResults(const MatchResults&) = delete;
  MatchResults& operator=(const MatchResults&) = delete;
  MatchResults(MatchResults&& o) {
    results = std::move(o.results);
    segments = std::move(o.segments);
    edges = std::move(o.edges);
    score = o.score;
    e1 = segments.empty() || segments.front().source < 1.0f ? edges.cbegin() : edges.cbegin() + 1;
    e2 = segments.empty() || segments.back().target > 0.0f ? edges.cend() : edges.cend() - 1;
  }
  MatchResults& operator=(MatchResults&& o) {
    results = std::move(o.results);
    segments = std::move(o.segments);
    edges = std::move(o.edges);
    score = o.score;
    e1 = segments.empty() || segments.front().source < 1.0f ? edges.cbegin() : edges.cbegin() + 1;
    e2 = segments.empty() || segments.back().target > 0.0f ? edges.cend() : edges.cend() - 1;
    return *this;
  }

  std::vector<MatchResult> results;
  std::vector<EdgeSegment> segments;
  std::vector<uint64_t> edges;
  float score;
  std::vector<uint64_t>::const_iterator e1;
  std::vector<uint64_t>::const_iterator e2;
};

} // namespace meili
} // namespace valhalla

#endif // MMP_MATCH_RESULT_H_
