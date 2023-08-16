

#ifndef NodeRfc_Log_H
#define NodeRfc_Log_H

#include <sapnwrfc.h>
#include <chrono>
#include <fstream>
#include <initializer_list>
#include <map>
#include <string>
#include "noderfc.h"

namespace node_rfc {

enum class logClass {
  client = 0,
  pool = 1,
  server = 2,
  throughput = 3,
  nwrfc = 4
};

enum class logLevel {
  none = 0,
  fatal = 1,
  error = 2,
  warning = 3,
  info = 4,
  debug = 5,
  all = 6
};

// Logger

class Log {
 private:
  // Log file name
  std::string log_fname;

  // Logging inactive by default
  std::map<logClass, logLevel> log_config = {
      {logClass::client, logLevel::none},
      {logClass::pool, logLevel::none},
      {logClass::server, logLevel::none},
      {logClass::throughput, logLevel::none},
      {logClass::nwrfc, logLevel::none}};

  // Log timestamp format
  long long timestamp();

 public:
  // Set log file name
  void set_log_file_name(const std::string& file_name);

  // Set log level
  void set_log_level(const logClass component_id, const logLevel log_level_id);
  void set_log_level(const logClass component_id,
                     const Napi::Value logLevelValue);

  // Default log filename
  explicit Log(std::string log_fname = "_noderfc.log");
  ~Log();

  // Write regular arguments. Must be defined in header becuse of variadic
  // interface
  template <typename... Args>
  void record(const logClass component_id,
              const logLevel log_level_id,
              Args&&... args) {
    if (log_level_id > log_config[component_id]) {
      return;
    }
    using namespace std;

    // Enum to string mapping
    const string component_names[] = {
        "client", "pool", "server", "throughput", "nwrfc"};
    const string severity_names[] = {
        "none", "fatal", "error", "warning", "info", "debug", "all"};
    time_t now = time(nullptr);

    // Write log message
    ofstream ofs;
    ofs.open(log_fname.c_str(), ofstream::out | ios::app);
    ofs << endl
        << endl
        << put_time(localtime(&now), "%F %T [") << timestamp() << "] >> "
        << component_names[static_cast<uint_t>(component_id)] << " ["
        << severity_names[static_cast<uint_t>(log_level_id)] << "] thread "
        << std::this_thread::get_id() << endl
        << "\t";
    (ofs << ... << args);
    ofs.close();
  }

  // Used for SAP unicode strings logging because the handle scope may
  // not be available for standard SAP unicode to Napi::String conversion
  void record(const logClass component_id,
              const logLevel log_level_id,
              const SAP_UC* message) {
    if (log_level_id > log_config[component_id]) {
      return;
    }
    FILE* fp = fopen(log_fname.c_str(), "a");
    fprintf(fp, "%s", "'");
    fprintfU(fp, message);
    fprintf(fp, "%s", "'");
    fclose(fp);
  }
};

}  // namespace node_rfc

#endif