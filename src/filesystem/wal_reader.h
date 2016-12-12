#pragma once
// seastar
#include <core/fstream.hh>
// generated
#include "flatbuffers/wal_generated.h"
// smf
#include "filesystem/wal_reader_node.h"


namespace smf {
class wal_reader {
  public:
  explicit wal_reader(sstring _directory, uint64_t epoch = 0)
    : directory(_directory), epoch_(epoch) {}

  wal_reader(wal_reader &&o)
    : directory(o.directory)
    , epoch_(o.epoch_)
    , current_(std::move(o.reader_)) {}

  /// brief - returns the next record in the log
  future<temporary_buffer<char>> next();

  future<> close();
  future<> open();
  ~wal_reader();
  const sstring directory;

  private:
  uint64_t epoch_;
  std::unique_ptr<wal_reader_node> reader_ = nullptr;
};

} // namespace smf
