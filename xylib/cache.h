// Public API of xylib library.
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id$

/// This header is new in 0.4 and may be changed in future.
/// Support for caching files read by xylib.
/// Usage is similar to load_file() from xylib.h:
///  shared_ptr<const xylib::DataSet> my_dataset = xylib::cached_load_file(...);
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
#include "xylib.h"

// MSVC has shared_ptr in <memory>, but let's use boost
#ifndef _MSC_VER
// the value here (1 or 0) is set by xylib configure script
// (yes, the configure script can modify this file!)
#define XYLIB_USE_TR1_MEMORY 1
#endif

#if XYLIB_USE_TR1_MEMORY
# include <tr1/memory>
  using std::tr1::shared_ptr;
#else
# include <boost/shared_ptr.hpp>
  using boost::shared_ptr;
#endif

namespace xylib
{

struct CacheImp;

// singleton
class XYLIB_API Cache
{
public:
    // get instance
    static Cache* Get()
        { if (instance_ == NULL) instance_ = new Cache(); return instance_; }

    // Arguments are the same as in load_file() in xylib.h,
    // but a const ref is returned instead of pointer.
    shared_ptr<const DataSet> load_file(std::string const& path,
                                        std::string const& format_name="",
                                        std::string const& options="");

    // set max. number of cached files, default=1
    void set_max_size(size_t max_size);
    // get max. number of cached files
    inline size_t get_max_size() const;

    // clear cache
    void clear_cache();

private:
    static Cache *instance_; // for singleton pattern
    CacheImp* imp_;
    Cache();
};

inline
shared_ptr<const DataSet> cached_load_file(std::string const& path,
                                           std::string const& format_name="",
                                           std::string const& options="")
{
    return Cache::Get()->load_file(path, format_name, options);
}


} // namespace xylib

#endif // XYLIB_CACHE_H_
