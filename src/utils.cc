#include "utils.h"

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

namespace util {
namespace file {

boost::posix_time::ptime modificationTime(const std::string &path_)
{
  try {
    fs::path path( path_ );

    if( ! fs::exists( path ) ) {
      return pt::ptime();
    } 
    std::time_t mt = fs::last_write_time(path);
    return pt::from_time_t(mt);
  }
  catch( const fs::filesystem_error &ex ) {
    throw std::runtime_error(ex.what());
  } 
  catch( ... ) {
    throw std::runtime_error("util::file::modificationTime: Exception: unknown error.");
  }
}
} // file
} // util


std::ostream &operator<<(std::ostream &o, milog::LogLevel ll) {
   switch( ll ) { 
      case milog::FATAL:  o << "FATAL"; break;
      case milog::ERROR:  o << "ERROR"; break;
      case milog::WARN:   o << "WARN"; break;
      case milog::INFO:   o << "INFO"; break;
      case milog::DEBUG:  o << "DEBUG"; break;
      case milog::DEBUG1: o << "DEBUG1"; break;
      case milog::DEBUG2: o << "DEBUG2"; break;
      case milog::DEBUG3: o << "DEBUG3"; break;
      case milog::DEBUG4: o << "DEBUG4"; break;
      case milog::DEBUG5: o << "DEBUG5"; break;
      case milog::DEBUG6: o << "DEBUG6"; break;
      default: 
       o << "NOTSET"; break;
   };
   return o;
}