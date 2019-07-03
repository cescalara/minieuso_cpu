#ifndef __MODULE_LOG__
#define __MODULE_LOG__

#include <sys/time.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#ifndef __APPLE__
#define LOG_DIR "/media/usb0/log"
#else
#define LOG_DIR "log"
#endif

/* function declarations */
std::string CreateLogname(void);

/**
 * simple logging class with different output levels and timestamp 
 */
class logstream: public std::ostringstream {
 public:
  /**
   * log level optiions
   */
  typedef
    enum
  {
    quiet,    
    error,    
    warning, 
    info,    
    all,     
    none,    
  } log_level;
 
  private:
  /**
   * stores the log level
   */
  log_level _m_log_level;
  /**
   * stores the current log level until the next flush/endl
   */
  log_level _m_current_level; /* only until the next flush/endl */

  /**
   * output stream
   */
  std::ostream & out;
     
 public:

  /**
   * print timestamp on flush
   */
  void flush() {
    if (_m_current_level <= _m_log_level) {
      struct timeval tv;
      gettimeofday(&tv, 0);
      time_t now = tv.tv_sec;
      struct tm * now_tm = localtime(&now);
      char buffer[40];
      strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", now_tm);
      out << buffer
	  << "." << std::setfill('0') 
	  << std::setw(6) << (tv.tv_usec) 
	  << std::setfill(' ') << ' '
	  << str();
      out.flush();
    }
    
    str("");
    _m_current_level = none;
  }
  
  template <typename T>
  inline logstream & operator<<( const T & t ) {
    (*(std::ostringstream*)this) << t;
    return * this;
  }
  
  /**
   * overload the << operator
   */
  inline logstream & operator<<(const log_level & level) { _m_current_level = level; return * this; } 
  
  /**
   * sets the acceptable message level 
   * until next flush/endl 
   */
  inline void set_level(log_level level) { _m_current_level = level; }
  inline log_level get_level() const { return _m_current_level; }
  
  inline void change_log_level(log_level level) { _m_log_level = level; }
  inline log_level get_log_lvel() const { return _m_log_level; }
     
  /*
    logstream(const char * filename, 
    log_level log_level=all,
    std::ios::openmode mode=std::ios::app)
    : _m_log_level(log_level),
    _m_current_level(none),
       out(filename,mode) {}
  */

  logstream ( std::ostream & log_stream,
	      log_level level = all)
    : _m_log_level(level),
      _m_current_level(none),
      out(log_stream){}
  
  
  virtual ~logstream() 
  { 
    flush();
  }
  
  /**
   * stubs for manipulators 
   */
  typedef logstream & (*logstream_manip)(logstream &);
  logstream & operator<<(logstream_manip manip) { return manip(*this); }
  
};

struct __logstream_level { logstream::log_level _m_level; };
inline __logstream_level setlevel(logstream::log_level _level) {
  __logstream_level level = { _level };
  return level;
}

inline logstream & operator<<(logstream & out, const __logstream_level & level ) {
  out.set_level(level._m_level);
  return out;
}

namespace std { inline logstream & endl(logstream & out) { out.put('\n'); out.flush(); return out; } }

/* external variables */
extern std::string log_name;
extern logstream clog;

#endif
/* __MODULE_LOG__ */
