

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
enum class logLevel { off = 0, error = 1, warning = 2, debug = 3 };

class Log {
 private:
  // Log file name
  std::string log_fname;

  // Component name, enum map
  std::map<std::string, logClass> component_name_to_enum = {
      {"client", logClass::client},
      {"pool", logClass::pool},
      {"server", logClass::server},
      {"throughput", logClass::throughput},
      {"nwrfc", logClass::nwrfc}};

  // Active logging components
  std::map<logClass, logLevel> log_config = {
      {logClass::client, logLevel::off},
      {logClass::pool, logLevel::off},
      {logClass::server, logLevel::off},
      {logClass::throughput, logLevel::off},
      {logClass::nwrfc, logLevel::off}};

  // Get component name for enum
  std::string get_component_name(const logClass component_id);

  long long timestamp();

 public:
  void set_log_level(const logClass component_id, const logLevel log_level_id);
  void set_log_level(const logClass component_id,
                     const Napi::Value logLevelValue);

  Log(std::string log_fname = "_noderfc.log");
  ~Log();

  // for regular arguments
  template <typename... Args>
  void write(const logClass component_id,
             const logLevel log_level_id,
             Args&&... args) {
    using namespace std;
    if (log_level_id > log_config[component_id]) {
      return;
    }

    const string severity_names[] = {"off", "error", "warning", "debug"};
    ofstream ofs;
    ofs.open(log_fname.c_str(), ofstream::out | ios::app);
    ofs << endl << endl;
    time_t now = time(nullptr);
    ofs << put_time(localtime(&now), "%F %T [") << timestamp() << "] >> ";
    ofs << get_component_name(component_id) << " ["
        << severity_names[static_cast<uint_t>(log_level_id)] << "] thread "
        << std::this_thread::get_id() << endl
        << "\t";
    (ofs << ... << args);
    ofs.close();
  }

  // Used for SAP unicode strings logging because the handle scope may
  // not be available for standard SAP unicode to Napi::String conversion
  void write(const logClass component_id,
             const logLevel log_level_id,
             const SAP_UC* message) {
    if (log_level_id > log_config[component_id]) {
      return;
    }
    FILE* fp = fopen(log_fname.c_str(), "a");
    fprintf(fp, "'");
    fprintfU(fp, message);
    fprintf(fp, "'");
    fclose(fp);
  }
};

}  // namespace node_rfc

#endif