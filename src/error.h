// Copyright 2014 SAP AG.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http: //www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

#ifndef ERROR_H_
#define ERROR_H_


using namespace v8;


Local<Object> RfcLibError(RFC_ERROR_INFO* errorInfo);
Local<Object> AbapError(RFC_ERROR_INFO* errorInfo);
Local<Value> wrapError(RFC_ERROR_INFO* errorInfo);

#endif
