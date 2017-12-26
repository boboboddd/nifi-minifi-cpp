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

#include "SenseHat.h"

#include <regex.h>
#include <curl/easy.h>
#include <uuid/uuid.h>
#include <memory>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "utils/ByteArrayCallback.h"
#include "core/FlowFile.h"
#include "core/logging/Logger.h"
#include "core/ProcessContext.h"
#include "core/Relationship.h"
#include "io/DataStream.h"
#include "io/StreamFactory.h"
#include "ResourceClaim.h"
#include "utils/StringUtils.h"

namespace org {
namespace apache {
namespace nifi {
namespace minifi {
namespace processors {

std::shared_ptr<utils::IdGenerator> SenseHAT::id_generator_ = utils::IdGenerator::getIdGenerator();

const char *SenseHAT::ProcessorName = "SenseHAT";

core::Relationship SenseHAT::Success("success", "All files are routed to success");

void SenseHAT::initialize() {
  logger_->log_info("Initializing SenseHAT");
  // Set the supported properties
  std::set<core::Property> properties;

  setSupportedProperties(properties);
  // Set the supported relationships
  std::set<core::Relationship> relationships;
  relationships.insert(Success);
  setSupportedRelationships(relationships);
}

void SenseHAT::onSchedule(const std::shared_ptr<core::ProcessContext> &context, const std::shared_ptr<core::ProcessSessionFactory> &sessionFactory) {

  imu = RTIMU::createIMU(&settings);
  if (imu) {
    imu->IMUInit();
    imu->setGyroEnable(true);
    imu->setAccelEnable(true);
  } else {
    throw std::runtime_error("RTIMU could not be initialized");
  }

  humidity_sensor_ = RTHumidity::createHumidity(&settings);
  if (humidity_sensor_) {
    humidity_sensor_->humidityInit();
  } else {
    throw std::runtime_error("RTHumidity could not be initialized");
  }

  pressure_sensor_ = RTPressure::createPressure(&settings);
  if (pressure_sensor_) {
    pressure_sensor_->pressureInit();
  } else {
    throw std::runtime_error("RTPressure could not be initialized");
  }

}

SenseHAT::~SenseHAT() {
delete humidity_sensor_;
delete pressure_sensor_;
}

void SenseHAT::onTrigger(const std::shared_ptr<core::ProcessContext> &context, const std::shared_ptr<core::ProcessSession> &session) {

auto flow_file_ = session->create();
flow_file_->setSize(0);

if ( imu->IMURead() ){
  RTIMU_DATA imuData = imu->getIMUData();
  auto vector = imuData.accel;
  std::string degrees = RTMath::displayDegrees("acceleration",vector);
  flow_file_->addAttribute("ACCELERATION", degrees);
}

RTIMU_DATA data;

bool have_sensor = false;

if (humidity_sensor_->humidityRead(data)) {
  if (data.humidityValid) {
    have_sensor = true;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << data.humidity;
    flow_file_->addAttribute("HUMIDITY", ss.str());
  }
}

if (pressure_sensor_->pressureRead(data)) {
  if (data.pressureValid) {
    have_sensor = true;
    {
      std::stringstream ss;
      ss << std::fixed << std::setprecision(2) << data.pressure;
      flow_file_->addAttribute("PRESSURE", ss.str());
    }

    if (data.temperatureValid){
      std::stringstream ss;
      ss << std::fixed << std::setprecision(2) << data.temperature;
      flow_file_->addAttribute("TEMPERATURE", ss.str());
    }

  }
}

if (have_sensor) {

  WriteCallback callback("SenseHAT");

  session->write(flow_file_,&callback);
  session->transfer(flow_file_, Success);
}

}

} /* namespace processors */
} /* namespace minifi */
} /* namespace nifi */
} /* namespace apache */
} /* namespace org */
