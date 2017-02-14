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

#ifndef LIBMINIFI_TEST_UNIT_REALTIMEDATACOLLECTOR_H_
#define LIBMINIFI_TEST_UNIT_REALTIMEDATACOLLECTOR_H_

#include "../../include/io/ClientSocket.h"
TEST_CASE("TestSocket", "[TestSocket1]") {

	Socket socket("localhost",8183);
	REQUIRE(-1 == socket.initialize() );
	REQUIRE("localhost" == socket.getHostname());

}

TEST_CASE("TestSocketWriteTest1", "[TestSocket2]") {

	Socket socket(Socket::getMyHostName(),8183);
	REQUIRE(-1 == socket.initialize() );

	socket.writeData(0,0);

	std::vector<uint8_t> buffer;
	buffer.push_back('a');

	REQUIRE(-1 == socket.writeData(buffer,1));


}

TEST_CASE("TestSocketWriteTest2", "[TestSocket3]") {

		std::vector<uint8_t> buffer;
	buffer.push_back('a');

	Socket server(Socket::getMyHostName(),9183,1);

	REQUIRE(-1 != server.initialize() );

	Socket client(Socket::getMyHostName(),9183);

	REQUIRE(-1 != client.initialize() );

	REQUIRE( 1 == client.writeData(buffer,1) );

	std::vector<uint8_t> readBuffer;
	readBuffer.resize(1);

	REQUIRE( 1== server.readData(readBuffer,1) );

	REQUIRE( readBuffer == buffer );



}

TEST_CASE("TestGetHostName", "[TestSocket4]") {

	REQUIRE( Socket::getMyHostName().length() > 0 );



}

#endif /* LIBMINIFI_TEST_UNIT_REALTIMEDATACOLLECTOR_H_ */
