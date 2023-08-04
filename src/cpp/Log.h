// SPDX-FileCopyrightText: 2014 SAP SE Srdjan Boskovic <srdjan.boskovic@sap.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NodeRfc_Log_H
#define NodeRfc_Log_H

#include <chrono>
#include <fstream>
#include <string>

namespace node_rfc {

#define SERVER_LOGFILE "_noderfc.log"

enum class logClass { client, pool, server, throughput };
enum class logSeverity { info, warning, error };

long long timestamp() {
  using namespace std;
  return chrono::duration_cast<chrono::milliseconds>(
             chrono::system_clock::now().time_since_epoch())
      .count();
}

template <typename... Args>
void _log(logClass component, logSeverity severity, Args&&... args) {
  using namespace std;
  string component_name;
  string severity_name;
  switch (component) {
    case logClass::client:
      component_name = "client";
      break;
    case logClass::server:
      component_name = "server";
      break;
    case logClass::pool:
      component_name = "pool";
      break;
    case logClass::throughput:
      component_name = "throughput";
      break;
    default:
      component_name = "component?";
  }
  switch (severity) {
    case logSeverity::info:
      severity_name = "info";
      break;
    case logSeverity::warning:
      severity_name = "warning";
      break;
    case logSeverity::error:
      severity_name = "error";
      break;
    default:
      severity_name = "severity?";
  }
  ofstream ofs;
  ofs.open(SERVER_LOGFILE, ofstream::out | ios::app);
  time_t now = time(nullptr);
  ofs << put_time(localtime(&now), "%F %T [") << timestamp() << "] ";
  ofs << component_name << " (" << severity_name << ") ";
  (ofs << ... << args);
  ofs << endl;
  ofs.close();
}

}  // namespace node_rfc
#endif