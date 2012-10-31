// Implementation of Public API of xylib library.
// Licence: Lesser GNU Public License 2.1 (LGPL)
// $Id$

#define BUILDING_XYLIB
#include "cache.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "xylib.h"
#include "util.h"

//no "using namespace std;" to avoid ambiguous symbol shared_ptr
using std::string;
using std::vector;

namespace {

// Returns last modification time of the file or 0 if error occurs.
// There is a function boost::filesystem::last_write_time(), but it requires
// linking with Boost.Filesystem library and this would cause more problems
// than it's worth.
// Portable libraries such as wxWidgets and Boost.Filesystem get mtime using
// ::GetFileTime() on MS Windows.
// Apparently some compilers also use _stat/_stat64 instead of stat.
// This will be implemented when portability problems are reported.
time_t get_file_mtime(string const& path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1)
        return 0;
    return sb.st_mtime;
}

struct CachedFile
{
    std::string path_;
    std::string format_name_;
    std::string options_;
    std::time_t read_time_;
    shared_ptr<const xylib::DataSet> dataset_;

    CachedFile(std::string const& path,
               std::string const& format_name,
               std::string const& options,
               shared_ptr<const xylib::DataSet> dataset)
        : path_(path), format_name_(format_name), options_(options),
          read_time_(std::time(NULL)), dataset_(dataset) {}
};

} // anonymous namespace

namespace xylib {

struct CacheImp
{
    size_t max_size_;
    std::vector<CachedFile> cache_;
};

Cache* Cache::instance_ = NULL;


Cache::Cache()
    : imp_(new CacheImp)
{
    imp_->max_size_ = 1;
}

// not thread-safe
shared_ptr<const DataSet> Cache::load_file(string const& path,
                                           string const& format_name,
                                           string const& options)
{
    std::vector<CachedFile>& cache_ = imp_->cache_;
    vector<CachedFile>::iterator iter;
    for (iter = cache_.begin(); iter < cache_.end(); ++iter) {
        if (path == iter->path_ && format_name == iter->format_name_
                && options == iter->options_) {
#if 1
            time_t mtime = get_file_mtime(path);
            if (mtime != 0 && mtime < iter->read_time_)
#else
            // if we can't check mtime, keep cache for 2 seconds
            if (time(NULL) - 2 < iter->read_time_)
#endif
                return iter->dataset_;
            else {
                cache_.erase(iter);
                break;
            }
        }
    }
    // this can throw exception
    shared_ptr<const DataSet> ds(xylib::load_file(path, format_name, options));

    if (cache_.size() >= imp_->max_size_)
        cache_.erase(cache_.begin());
    cache_.push_back(CachedFile(path, format_name, options, ds));
    return ds;
}

void Cache::set_max_size(size_t max_size)
{
    std::vector<CachedFile>& cache_ = imp_->cache_;
    imp_->max_size_ = max_size;
    if (max_size > cache_.size())
        cache_.erase(cache_.begin() + max_size, cache_.end());
}

size_t Cache::get_max_size() const
{
    return imp_->max_size_;
}

void Cache::clear_cache()
{
    imp_->cache_.clear();
}

} // namespace xylib

