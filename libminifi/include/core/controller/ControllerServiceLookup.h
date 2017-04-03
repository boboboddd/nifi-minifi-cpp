/**
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
#ifndef LIBMINIFI_INCLUDE_CORE_CONTROLLER_CONTROLLERSERVICELOOKUP_H_
#define LIBMINIFI_INCLUDE_CORE_CONTROLLER_CONTROLLERSERVICELOOKUP_H_

#include <map>
#include "core/core.h"
#include "core/ConfigurableComponent.h"
#include "ControllerService.h"

namespace org {
namespace apache {
namespace nifi {
namespace minifi {
namespace core {
namespace controller {

class ControllerServiceLookup : public CoreComponent, public ConfigurableComponent {
 public:

  ControllerServiceLookup()
      : CoreComponent(core::getClassName<ControllerService>()) {

  }

 protected:
  //std::map<std::string,std::>
};

} /* namespace controller */
} /* namespace core */
} /* namespace minifi */
} /* namespace nifi */
} /* namespace apache */
} /* namespace org */



#endif /* LIBMINIFI_INCLUDE_CORE_CONTROLLER_CONTROLLERSERVICELOOKUP_H_ */
