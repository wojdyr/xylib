// Public API of xylib library.
// Licence: Lesser GNU Public License 2.1 (LGPL)

/// Support for caching files read by xylib.
/// Usage is similar to load_file() from xylib.h:
///  std::shared_ptr<const xylib::DataSet> my_dataset = xylib::cached_load_file(...);
/// or
///  xylib::DataSet const& my_dataset = xylib::Cache::Get()->load_file(...);

#ifndef XYLIB_CACHE_H_
#define XYLIB_CACHE_H_

#ifndef __cplusplus
#error "xylib/cache.h is a C++ only header."
#endif

#include <ctime>
#include <string>
#include <vector>
#include <memory>  // for shared_ptr
#include "xylib.h"

namespace xylib
{

using dataset_shared_ptr = std::shared_ptr<const xylib::DataSet>;

struct CacheImp;

// singleton
class XYLIB_API Cache
{
public:
    // get instance
    static Cache* Get();

    // Arguments are the same as in load_file() in xylib.h,
    // but a const ref is returned instead of pointer.
    dataset_shared_ptr load_file(std::string const& path,
                                 std::string const& format_name="",
                                 std::string const& options="");

    // set max. number of cached files, default=1
    void set_max_size(size_t max_size);
    // get max. number of cached files
    size_t get_max_size() const;

    // clear cache
    void clear_cache();

private:
    static Cache *instance_; // for singleton pattern
    CacheImp* imp_;
    Cache();
    ~Cache();
    Cache(const Cache&); // disallow
};

inline
dataset_shared_ptr cached_load_file(std::string const& path,
                                    std::string const& format_name="",
                                    std::string const& options="")
{
    return Cache::Get()->load_file(path, format_name, options);
}

} // namespace xylib

// deprecated, for compatibility with xylib 1.6
using xylib::dataset_shared_ptr;

#endif // XYLIB_CACHE_H_
