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

#include <regex>
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

#include "core/FlowFile.h"
#include "core/logging/Logger.h"
#include "core/ProcessContext.h"
#include "core/Relationship.h"
#include "ExecuteJavaClass.h"
#include "ResourceClaim.h"
#include "utils/StringUtils.h"
#include "utils/ByteArrayCallback.h"

namespace org {
namespace apache {
namespace nifi {
namespace minifi {
namespace processors {

core::Property ExecuteJavaClass::JVMControllerService(
    core::PropertyBuilder::createProperty("JVM Controller Service")->withDescription("Name of controller service defined within this flow")->isRequired(true)->withDefaultValue<std::string>("")->build());
core::Property ExecuteJavaClass::NiFiProcessor(
    core::PropertyBuilder::createProperty("NiFi Processor")->withDescription("Name of NiFi processor to load and run")->isRequired(true)->withDefaultValue<std::string>("")->build());

core::Property ExecuteJavaClass::NarDirectory(
    core::PropertyBuilder::createProperty("Nar Directory")->withDescription("Directory used by NarClassloader")->isRequired(true)->withDefaultValue<std::string>("")->build());

const char *ExecuteJavaClass::ProcessorName = "ExecuteJavaClass";

core::Relationship ExecuteJavaClass::Success("success", "All files are routed to success");
void ExecuteJavaClass::initialize() {
  logger_->log_info("Initializing ExecuteJavaClass");
  // Set the supported properties
  std::set<core::Property> properties;
  properties.insert(JVMControllerService);
  properties.insert(NiFiProcessor);
  properties.insert(NarDirectory);
  setSupportedProperties(properties);
  setAcceptAllProperties();
  // Set the supported relationships
  std::set<core::Relationship> relationships;
  relationships.insert(Success);
  setSupportedRelationships(relationships);
}

void ExecuteJavaClass::onSchedule(const std::shared_ptr<core::ProcessContext> &context, const std::shared_ptr<core::ProcessSessionFactory> &sessionFactory) {
  std::string controller_service_name;

  if (getProperty(JVMControllerService.getName(), controller_service_name)) {
    auto cs = context->getControllerService(controller_service_name);
    if (cs == nullptr) {
      throw std::runtime_error("Could not load controller service");
    }
    java_servicer_ = std::static_pointer_cast<controllers::JavaControllerService>(cs);

  } else {
    throw std::runtime_error("Could not load controller service");
  }

  if (!getProperty(NiFiProcessor.getName(), class_name_)) {
    throw std::runtime_error("NiFi Processor must be defined");
  }

  std::string dir_name_;

  if (!getProperty(NarDirectory.getName(), dir_name_)) {
    throw std::runtime_error("NiFi Directory must be defined");
  }

  std::cout << "ahh" << std::endl;

  spn = java_servicer_->loadClass("org/apache/nifi/processor/JniProcessContext");

  std::cout << "now nar " << std::endl;
  narClassLoaderClazz = java_servicer_->loadClass("org/apache/nifi/processor/JniClassLoader");

  loader_ = std::unique_ptr<NarClassLoader>(new NarClassLoader(java_servicer_, narClassLoaderClazz, dir_name_));

  spn = java_servicer_->loadClass("org/apache/nifi/processor/JniProcessContext");

  init = java_servicer_->loadClass("org/apache/nifi/processor/JniInitializationContext");

  auto obj = spn.newInstance();

  auto initializer = init.newInstance();

  // create provided class

  std::cout << "ah" << std::endl;
  clazzInstance = loader_->newInstance(class_name_);
  auto onScheduledName = loader_->getAnnotation(class_name_,"OnScheduled");
  std::cout << "ah2 " << (clazzInstance == nullptr) << " " << onScheduledName << std::endl;
  //auto clazz = java_servicer_->loadClass(class_name_);
  current_processor_class = java_servicer_->getObjectClass(class_name_, clazzInstance);
  // attempt to schedule here
  std::cout << "ah3" << std::endl;
//  context->setDynamicProperty("Unique FlowFiles", "false");
//  context->setDynamicProperty("character-set", "UTF-8");
//  context->setDynamicProperty("Batch Size", "7");

  JNINativeMethod methods[] = { { "getPropertyValue", "(Ljava/lang/String;)Ljava/lang/String;", reinterpret_cast<void*>(&Java_org_apache_nifi_processor_JniProcessContext_getPropertyValue) }

  };
  spn.registerMethods(methods, 1);
  std::cout << "ah3" << std::endl;
  java_servicer_->setReference<core::ProcessContext>(obj, context.get());
  std::cout << "ah4" << std::endl;
  current_processor_class.callVoidMethod(clazzInstance, "initialize", "(Lorg/apache/nifi/processor/ProcessorInitializationContext;)V", initializer);
  std::cout << "calling" << std::endl;
  try {
    current_processor_class.callVoidMethod(clazzInstance, onScheduledName.c_str(), "(Lorg/apache/nifi/processor/ProcessContext;)V", obj);
  } catch (std::runtime_error &re) {
    // this is avoidable.
  }
  std::cout << "called" << std::endl;

}

ExecuteJavaClass::~ExecuteJavaClass() {
}

void ExecuteJavaClass::onTrigger(const std::shared_ptr<core::ProcessContext> &context, const std::shared_ptr<core::ProcessSession> &session) {
  std::cout << "lolwut" << std::endl;
  // need env reference since we are in a sparate thread
  auto java_process_context = spn.newInstance(java_servicer_->attach());
  java_servicer_->setReference<core::ProcessContext>(java_process_context, context.get());

  auto sessioncls = java_servicer_->loadClass("org/apache/nifi/processor/JniProcessSession");
  JNINativeMethod methods[] = { { "create", "()Lorg/apache/nifi/flowfile/FlowFile;", reinterpret_cast<void*>(&Java_org_apache_nifi_processor_JniProcessSession_create) }, { "get",
      "()Lorg/apache/nifi/flowfile/FlowFile;", reinterpret_cast<void*>(&Java_org_apache_nifi_processor_JniProcessSession_get) },
      { "write", "(Lorg/apache/nifi/flowfile/FlowFile;[B)Z",
                     reinterpret_cast<void*>(&Java_org_apache_nifi_processor_JniProcessSession_write) },
      { "transfer","(Lorg/apache/nifi/flowfile/FlowFile;Ljava/lang/String;)V", reinterpret_cast<void*>(&Java_org_apache_nifi_processor_JniProcessSession_transfer) }

  };
  sessioncls.registerMethods(methods, 4);

  try {

    auto java_process_session = sessioncls.newInstance(java_servicer_->attach());

    java_servicer_->setReference<core::ProcessSession>(java_process_session, session.get());

    current_processor_class.callVoidMethod(java_servicer_->attach(), clazzInstance, "onTrigger", "(Lorg/apache/nifi/processor/ProcessContext;Lorg/apache/nifi/processor/ProcessSession;)V",
                                           java_process_context, java_process_session);
  } catch (const std::exception &e ) {
    std::cout << "Oh " << class_name_ << "  error " << e.what() << std::endl;
  }
  std::cout << "ohsnap" << std::endl;

}

}
/* namespace processors */
} /* namespace minifi */
} /* namespace nifi */
} /* namespace apache */
} /* namespace org */

