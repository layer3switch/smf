#pragma once
// seastar
#include <core/fstream.hh>
// generated
#include "flatbuffers/wal_generated.h"

namespace smf {
// TODO(agallego) - missing stats

/// \brief - given a starting prefix:
/// smf0_*
/// it will iterate through the records on the file for that core which in
/// the case of smf0_...wal - would be core 0
///
class wal_reader_node {
  public:
  wal_reader_node(sstring _directory);
  future<temporary_buffer<char>> next();
  /// \brief flushes the file before closing
  future<> close();
  future<> open();
  ~wal_reader_node();

  const sstring prefix_name;



  private:
  input_stream<char> ifstream_;
};

} // namespace smf
