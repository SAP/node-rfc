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

enum logClass { client = 0, pool, server, throughput };
enum logSeverity { info = 0, warning, error };

long long timestamp() {
  using namespace std;
  return chrono::duration_cast<chrono::milliseconds>(
             chrono::system_clock::now().time_since_epoch())
      .count();
}

template <typename... Args>
void _log(logClass component_id, logSeverity severity_id, Args&&... args) {
  using namespace std;
  const string component_names[4] = {"client", "pool", "server", "throughput"};
  const string severity_names[3] = {"info", "warning", "error"};
  ofstream ofs;
  ofs.open(SERVER_LOGFILE, ofstream::out | ios::app);
  time_t now = time(nullptr);
  ofs << put_time(localtime(&now), "%F %T [") << timestamp() << "] ";
  ofs << component_names[component_id] << " (" << severity_names[severity_id]
      << ") ";
  (ofs << ... << args);
  ofs << endl;
  ofs.close();
}

}  // namespace node_rfc
#endif