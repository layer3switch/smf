// source header
#include "filesystem/wal_reader.h"
// third party
#include <core/reactor.hh>
// smf
#include "log.h"
#include "filesystem/wal_reader_node.h"
#include "filesystem/wal_head_file_functor.h"


namespace smf {

future<> wal_reader::close() {
  assert(reader_ != nullptr);
  return reader_->close();
}

future<> wal_reader::open() {
  return open_directory(directory).then([this](file f) {
    auto l = make_lw_shared < wal_head_file_functor
             < [epoch = epoch_](auto a, auto b) {
      auto is_less_than = a != b && !(a < b);
      // TODO(keep the epoch index here)
      return is_less_than;
    }
    >> (std::move(f));
    return l->close().then([l, this]() mutable {
      if(l->last_file.empty()) {
        LOG_DEBUG("Empty dir. Creating first WAL");
        auto id = to_sstring(engine().cpu_id());
        reader_ = std::make_unique<wal_writer_node>(l->name_parser.prefix + id);
        return reader_->open();
      }
      LOG_DEBUG("Reading last file epoch");
      return open_file_dma(l->last_file, open_flags::ro)
        .then([this, prefix = l->name_parser.prefix](file last) {
          auto lastf = make_lw_shared<file>(std::move(last));
          return lastf->size().then([this, prefix, lastf](uint64_t size) {
            auto id = to_sstring(engine().cpu_id());
            reader_ = std::make_unique<wal_writer_node>(prefix + id, size);
            return lastf->close().then([this] { return reader_->open(); });
          });
        });
    });
  });
}

future<temporary_buffer<char>> wal_reader::next() { return reader_->next(); }


} // namespace smf
