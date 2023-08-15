#include "Log.h"

namespace node_rfc {

// Get component name for enum
std::string Log::get_component_name(const logClass component_id) {
  for (auto it = component_name_to_enum.begin();
       it != component_name_to_enum.end();
       ++it)
    if (it->second == component_id) return it->first;
  return "not_found";
}

long long Log::timestamp() {
  using namespace std;
  return chrono::duration_cast<chrono::milliseconds>(
             chrono::system_clock::now().time_since_epoch())
      .count();
}

void Log::set_log_level(const logClass component_id,
                        const logLevel log_level_id) {
  log_config[component_id] = log_level_id;
}

void Log::set_log_level(const logClass component_id,
                        const Napi::Value logLevelValue) {
  char errmsg[ERRMSG_LENGTH];
  if (!logLevelValue.IsNumber()) {
    snprintf(errmsg,
             ERRMSG_LENGTH - 1,
             "Logging level is not supported: \"%s\"",
             logLevelValue.ToString().Utf8Value().c_str());
    Napi::TypeError::New(logLevelValue.Env(), errmsg)
        .ThrowAsJavaScriptException();
    return;
  }

  uint_t log_level = static_cast<uint_t>(logLevelValue.As<Napi::Number>());

  if (log_level != static_cast<uint_t>(logLevel::off) &&
      log_level != static_cast<uint_t>(logLevel::error) &&
      log_level != static_cast<uint_t>(logLevel::warning) &&
      log_level != static_cast<uint_t>(logLevel::debug)) {
    snprintf(errmsg,
             ERRMSG_LENGTH - 1,
             "Logging level not supported: \"%s\"",
             logLevelValue.ToString().Utf8Value().c_str());
    Napi::TypeError::New(logLevelValue.Env(), errmsg)
        .ThrowAsJavaScriptException();
    return;
  }
  set_log_level(component_id, (logLevel)log_level);
}

Log::Log(std::string log_fname) : log_fname(log_fname) {
  std::ofstream ofs;
  ofs.open(log_fname, std::ofstream::out | std::ofstream::trunc);
  ofs.close();
}

Log::~Log() {}

Log _log = Log();

}  // namespace node_rfc
