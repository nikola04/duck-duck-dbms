#pragma once

#include "duck/common/config.hpp"
#include "duck/common/types.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace duck {

struct [[gnu::packed]] DiskHeader {
    std::uint64_t magic;
    std::uint64_t version;
    std::uint64_t capacity;
    std::uint64_t free_list_size;
};

class DiskManager {
public:
    explicit DiskManager(std::string path);
    ~DiskManager();

    void read_page(PageID page_id, std::span<std::byte> buffer);
    void write_page(PageID page_id, std::span<const std::byte> buffer);

    PageID allocate_page();
    void deallocate_page(PageID page_id);

    size_t capacity() const {
        return capacity_;
    }

    void flush_all();

private:
    const std::string path_;
    const int fd_;
    DiskHeader disk_header_{kDB_MAGIC, kDB_FORMAT_VERSION, 0, 0};

    std::atomic<PageID> capacity_{0};

    std::vector<PageID> free_list_{};
    std::mutex free_list_mutex{};

    off_t get_size() const;

    void init_header();
};

} // namespace duck