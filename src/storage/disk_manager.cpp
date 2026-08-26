#include "duck/storage/disk_manager.hpp"
#include "duck/common/config.hpp"
#include "duck/common/types.hpp"
#include "duck/config/defaults.hpp"
#include "duck/config/sizes.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <format>
#include <mutex>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace duck {

DiskManager::DiskManager(std::string path)
    : path_{std::move(path)}, fd_{open(path_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)} {

    if (fd_ == -1) {
        throw std::invalid_argument("Disk file failed to open: " + path_);
    }

    if (off_t size{get_size()}; size > 0) {
        capacity_ = size / kPAGE_SIZE;
    }

    if (capacity_ == 0) {
        allocate_page();
        flush_all();
    } else
        init_header();

    if (disk_header_.magic != kDB_MAGIC)
        throw std::runtime_error("DiskManager::DiskManager: Database file is not recognized by DUCKDB.");

    if (disk_header_.version != kDB_FORMAT_VERSION)
        throw std::runtime_error("DiskManager::DiskManager: Database file version not supported.");
}

DiskManager::~DiskManager() {
    if (fd_ == -1)
        return;

    flush_all();
    close(fd_);
}

PageID DiskManager::allocate_page() {
    if (std::lock_guard lock{free_list_mutex}; !free_list_.empty()) {
        PageID page_id{free_list_.back()};
        free_list_.pop_back();
        return page_id;
    }

    return capacity_.fetch_add(1);
}

void DiskManager::deallocate_page(PageID page_id) {
    if (page_id == kDISK_METADATA_PAGE_ID)
        throw std::runtime_error("DiskManager::deallocate_page: deallocating disk meta page is not allowed");

    std::lock_guard lock{free_list_mutex};

    if (free_list_.size() > 1024) {
        std::println(
            "WARNING - DiskManager::deallocate_page: free list size exceeds 1024 and deallocated page wont be reused");
        return;
    }

    free_list_.push_back(page_id);
}

void DiskManager::flush_all() {
    std::lock_guard lock{free_list_mutex};

    // write header
    disk_header_.free_list_size = free_list_.size();
    auto header_ptr{static_cast<void*>(&disk_header_)};
    pwrite(fd_, header_ptr, sizeof(DiskHeader), 0);

    // write list
    pwrite(fd_, free_list_.data(), free_list_.size() * sizeof(PageID), sizeof(DiskHeader));
}

void DiskManager::write_page(PageID page_id, std::span<const std::byte> buffer) {
    if (page_id >= capacity_)
        throw std::runtime_error("DiskManager: trying to write into not allocated page: " + std::to_string(page_id));

    if (buffer.size() < kPAGE_SIZE)
        throw std::runtime_error(
            std::format("DiskManager: write buffer is smaller than page: {}; buffer size: {}", page_id, buffer.size()));

    size_t offset{page_id * kPAGE_SIZE};
    ssize_t bytes_written{pwrite(fd_, buffer.data(), kPAGE_SIZE, offset)};

    if (bytes_written != static_cast<ssize_t>(kPAGE_SIZE))
        throw std::runtime_error("DiskManager: incomplete write for page: " + std::to_string(page_id));
}

void DiskManager::read_page(PageID page_id, std::span<std::byte> buffer) {
    if (page_id >= capacity_)
        throw std::runtime_error("DiskManager: trying to read not allocated page: " + std::to_string(page_id));

    if (buffer.size() < kPAGE_SIZE)
        throw std::runtime_error(
            std::format("DiskManager: read buffer is smaller than page: {}; buffer size: {}", page_id, buffer.size()));

    size_t offset{page_id * kPAGE_SIZE};
    ssize_t bytes_read{pread(fd_, buffer.data(), kPAGE_SIZE, offset)};

    if (bytes_read < 0)
        throw std::runtime_error("DiskManager: incomplete read for page: " + std::to_string(page_id));

    // fill rest of the bytes with 0 for most common case where whole page can be empty
    if (static_cast<size_t>(bytes_read) < kPAGE_SIZE) {
        std::fill_n(buffer.data() + bytes_read, kPAGE_SIZE - bytes_read, std::byte{0});
    }
}

off_t DiskManager::get_size() const {
    if (fd_ == -1)
        return -1;
    struct stat sb{};
    if (fstat(fd_, &sb) == -1) {
        return -1;
    }
    return sb.st_size;
}

void DiskManager::init_header() {
    std::vector<std::byte> buffer(kPAGE_SIZE);
    read_page(kDISK_METADATA_PAGE_ID, buffer);

    disk_header_ =
        std::bit_cast<DiskHeader>(*reinterpret_cast<std::array<std::byte, sizeof(DiskHeader)>*>(buffer.data()));

    free_list_.resize(disk_header_.free_list_size);
    std::memcpy(free_list_.data(), buffer.data() + sizeof(DiskHeader), disk_header_.free_list_size * sizeof(PageID));
}

} // namespace duck