

#ifndef NodeRfc_Log_H
#define NodeRfc_Log_H

#include <sapnwrfc.h>
#include <chrono>
#include <fstream>
#include <set>
#include <string>
#include "noderfc.h"

#define LOG_FILE_NAME "_noderfc.log"

namespace node_rfc {

enum logClass { client = 0, pool, throughput, server };
enum logSeverity { off = 0, error, warning, info };

typedef struct _LogConfig {
  std::set<logClass> log_class = {};
  uint_t log_severity = logSeverity::off;

} LogConfig;

typedef struct _ServerOptions {
  LogConfig logging = LogConfig();

  ~_ServerOptions() {}
} ServerOptions;

class Log {
 private:
  std::string log_fname;
  LogConfig log_config = LogConfig();

 public:
  long long timestamp() {
    using namespace std;
    return chrono::duration_cast<chrono::milliseconds>(
               chrono::system_clock::now().time_since_epoch())
        .count();
  }

  Log(std::string log_fname = LOG_FILE_NAME) : log_fname(log_fname) {
    std::ofstream ofs;
    ofs.open(log_fname, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
  }

  ~Log() {}

  // for regular arguments
  template <typename... Args>
  void write(logClass component_id, logSeverity severity_id, Args&&... args) {
    using namespace std;
    const string component_names[4] = {
        "client", "pool", "server", "throughput"};
    const string severity_names[3] = {"info", "warning", "error"};
    ofstream ofs;
    ofs.open(log_fname.c_str(), ofstream::out | ios::app);
    ofs << endl << endl;
    time_t now = time(nullptr);
    ofs << put_time(localtime(&now), "%F %T [") << timestamp() << "] >> ";
    ofs << component_names[component_id] << " [" << severity_names[severity_id]
        << "] thread " << std::this_thread::get_id() << endl
        << "\t";
    (ofs << ... << args);
    ofs.close();
  }

  // for SAP unicode string, only as the last one in log oputput line
  void write(SAP_UC const* message) {
    FILE* fp = fopen(log_fname.c_str(), "a");
    fprintf(fp, "'");
    fprintfU(fp, message);
    fprintf(fp, "'");
    fclose(fp);
  }
};

}  // namespace node_rfc

#endif