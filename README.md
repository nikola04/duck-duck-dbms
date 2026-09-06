# Duck Duck DBMS

A simple thread-safe Database Managment system build in C++23

## Architecture

Layered, bottom-up:

- **`duck_storage`** — `DiskManager`: raw page I/O (`pread`/`pwrite`), page allocation/deallocation with free-list reuse.
- **`duck_buffer`** — `BufferPoolManager`: fixed-size RAM cache over disk pages, LRU-based eviction, per-page latches (fine-grained locking, independent of the global buffer-pool latch), `PinnedPage` RAII wrapper.
- **`duck_tuple`** — `SlottedPage` (on-disk page layout for variable-length tuples), `Value`/`Schema`/`Column`/`Tuple` (typed row representation with serialization to/from raw bytes, NULL bitmap, fixed vs. variable-length column handling).
- **`duck_table`** — `TableHeap` (multi-page tuple storage with page-chain traversal), `Table` (schema-aware wrapper over `TableHeap`), cursor-style `Scan` for sequential iteration.
- **`duck_catalog`** — `Catalog`: persistent table registry. Bootstraps itself as an ordinary table (via `TableHeap`) rooted at a fixed, well-known page id, storing `{table_name, first_page_id, schema_bytes}` rows for every user-created table. On startup, scans its own page chain to reconstruct in-memory `Table` handles.

Each layer only depends on the one below it; concurrency guarantees (thread-safety, no data races) are verified independently at each layer under ThreadSanitizer.

### Known limitations

- **`Catalog::DropTable` is not safe against concurrent use of the dropped table.** If another thread holds a `Table*` obtained via an earlier `GetTable` call (e.g. mid-scan) while `DropTable` runs, that pointer becomes dangling once the corresponding entry is erased from the catalog's internal map — this is a use-after-free on the handle itself, distinct from (and not prevented by) the BPM's existing pin/latch protection over page _contents_. Safe concurrent DDL requires reference-counted table handles or DDL/DML locking via a Lock Manager, intentionally out of scope for this layer and planned as part of a future Concurrency Control layer.
- **`DropTable` page reclamation is best-effort.** `TableHeap`/`Table::drop_pages()` walks the full page chain and attempts to delete every page via the buffer pool, but a page still pinned by a concurrent operation cannot be reclaimed; it is simply left allocated (leaked) rather than retried, and the count of unreclaimed pages is returned to the caller rather than raising an error. The catalog entry is still removed either way, so the table becomes logically inaccessible even if some of its pages remain on disk.
- **Cross-table RID access is not validated.** `TableHeap::GetTuple`/`DeleteTuple` fetch a page directly via `RID.page_id` without checking that the page actually belongs to this heap's page chain. Validating this would require walking the chain (defeating the O(1) purpose of a RID) or adding a `table_id` tag to `PageHeader`. For now this is a trust boundary enforced by upper layers (Catalog/Executor), not by `TableHeap` itself — trading correctness guarantees for O(1) get/delete performance.
- **Flush during eviction holds the global BPM latch.** `BufferPoolManager::swap_page` performs the dirty-page flush (disk write) while still holding the global `latch_`, serializing all other BPM operations during that I/O. The read of the incoming page is correctly done outside the lock; splitting the flush the same way (reserve-then-release-then-flush) is a known follow-up optimization.
- **No length validation on `Value` construction.** `Value::of(std::string)` accepts strings of any length regardless of the target column's `CHAR`/`VARCHAR` constraint. Validation happens later, at `Tuple::Serialize()` time (which has access to the `Schema`), not at `Value` construction — by design, since a `Value` can exist independently of any target column.
- **Persistent free-list, capped at one metadata page.** `DiskManager` persists its free-list (deallocated `page_id`s) to a reserved metadata page (`page_id = 0`) on flush, and reloads it on startup — deallocated pages are correctly recycled across process restarts. The list is capped at `kMAX_FREE_LIST_ENTRIES` (~1000 entries for a 4KB page); exceeding this currently throws rather than silently truncating. A natural extension would let the metadata header link to additional overflow pages (the same page-chaining technique used by `TableHeap`) once a workload accumulates enough deallocated pages to hit this limit.
- **DiskManager is POSIX-only.** Uses `pread`/`pwrite` directly; no Windows (`ReadFile`/`WriteFile` + `OVERLAPPED`) backend.
- **`reinterpret_cast` over raw page bytes (`PageHeader`, `Slot`) is technically in a strict-aliasing grey area** pre-C++23 `start_lifetime_as`. In practice this is the standard, universally-used approach for on-disk binary formats and works reliably on GCC/Clang for standard-layout structs, but it isn't formally guaranteed by the standard.
- **Deadlock avoidance** is timeout-based (1s), not wait-for-graph detection

- **RID-level locking currently does not protect deleted-slot reuse** by concurrent `INSERT` operations. An `INSERT` can reuse a slot belonging to another active transaction because the RID does not exist until after the tuple is inserted.
  e.g. it is dangerous when transaction is rollbacking and another already inserted into that RID.
  This should be fixed later by introducing page/slot-level locking or deferred slot reuse until the deleting transaction commits or aborts.

## License

This project is licensed under the [MIT License](./LICENSE).
