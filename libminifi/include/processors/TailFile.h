/**
 * @file TailFile.h
 * TailFile class declaration
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __TAIL_FILE_H__
#define __TAIL_FILE_H__

#include "FlowFileRecord.h"
#include "core/Processor.h"
#include "core/ProcessSession.h"
#include "core/core.h"

namespace org {
namespace apache {
namespace nifi {
namespace minifi {
namespace processors {

// TailFile Class
class TailFile : public core::Processor {
 public:
  // Constructor
  /*!
   * Create a new processor
   */
  TailFile(std::string name, uuid_t uuid = NULL)
      : core::Processor(name, uuid) {
    logger_ = logging::Logger::getLogger();
    _stateRecovered = false;
  }
  // Destructor
  virtual ~TailFile() {
    storeState();
  }
  // Processor Name
  static const std::string ProcessorName;
  // Supported Properties
  static core::Property FileName;
  static core::Property StateFile;
  // Supported Relationships
  static core::Relationship Success;

 public:
  // OnTrigger method, implemented by NiFi TailFile
  virtual void onTrigger(
      core::ProcessContext *context,
      core::ProcessSession *session);
  // Initialize, over write by NiFi TailFile
  virtual void initialize(void);
  // recoverState
  void recoverState();
  // storeState
  void storeState();

 protected:

 private:
  // Logger
  std::shared_ptr<logging::Logger> logger_;
  // File to save state
  std::string _stateFile;
  // State related to the tailed file
  std::string _currentTailFileName;
  uint64_t _currentTailFilePosition;
  bool _stateRecovered;
  uint64_t _currentTailFileCreatedTime;
  // Utils functions for parse state file
  std::string trimLeft(const std::string& s);
  std::string trimRight(const std::string& s);
  void parseStateFileLine(char *buf);
  void checkRollOver(std::string, std::string);

};

// Matched File Item for Roll over check
typedef struct {
  std::string fileName;
  uint64_t modifiedTime;
} TailMatchedFileItem;

} /* namespace processors */
} /* namespace minifi */
} /* namespace nifi */
} /* namespace apache */
} /* namespace org */

#endif
