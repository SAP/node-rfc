#include "Log.h"

namespace node_rfc {

long long Log::timestamp() {
  using namespace std;
  return chrono::duration_cast<chrono::milliseconds>(
             chrono::system_clock::now().time_since_epoch())
      .count();
}

void Log::set_log_file_name(const std::string& file_name) {
  log_fname = file_name;
}

void Log::set_log_level(const logClass component_id,
                        const logLevel log_level_id) {
  log_config[component_id] = log_level_id;
}

void Log::set_log_level(const logClass component_id,
                        const Napi::Value logLevelValue) {
  if (!logLevelValue.IsNumber()) {
    Napi::TypeError::New(logLevelValue.Env(),
                         "Logging level not  a number: \"" +
                             logLevelValue.ToString().Utf8Value() + "\"")
        .ThrowAsJavaScriptException();
    return;
  }

  // Check log level provided by application
  logLevel log_level =
      static_cast<logLevel>(logLevelValue.As<Napi::Number>().Int32Value());
  if (log_level < logLevel::none || log_level > logLevel::all) {
    Napi::TypeError::New(logLevelValue.Env(),
                         "Logging level not supported: \"" +
                             logLevelValue.ToString().Utf8Value() + "\"")
        .ThrowAsJavaScriptException();
    return;
  }

  // set log level
  set_log_level(component_id, log_level);
}

Log::Log(std::string log_fname) : log_fname(log_fname) {
  std::ofstream ofs;
  ofs.open(log_fname, std::ofstream::out | std::ofstream::trunc);
  ofs.close();
}

Log::~Log() {}

// Create logger instance
Log _log = Log();

}  // namespace node_rfc
