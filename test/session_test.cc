/** Copyright 2020-2021 Alibaba Group Holding Limited.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <memory>
#include <string>
#include <thread>

#include "arrow/api.h"
#include "arrow/io/api.h"

#include "basic/ds/tensor.h"
#include "client/client.h"
#include "client/ds/object_meta.h"
#include "common/util/logging.h"

using namespace vineyard;  // NOLINT(build/namespaces)

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage ./session_test <ipc_socket>");
    return 1;
  }
  std::string ipc_socket = std::string(argv[1]);

  {  // test basic functionality;
    Client client;
    VINEYARD_CHECK_OK(client.Open(ipc_socket));
    LOG(INFO) << "Connected to IPCServer: " << client.IPCSocket();

    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder(client, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder.Seal(client));
    ObjectID id = sealed_double_array->id();
    ObjectID copied_id = InvalidObjectID();
    VINEYARD_CHECK_OK(client.ShallowCopy(id, copied_id));
    CHECK(copied_id != InvalidObjectID());

    auto arrays = client.GetObjects({id, copied_id});
    CHECK_EQ(arrays.size(), 2);
    CHECK_EQ(arrays[0]->id(), id);
    CHECK_EQ(arrays[1]->id(), copied_id);
    LOG(INFO) << "Passed session creat/get test...";

    client.Disconnect();
  }

  {  // test isolation;
    Client client1, client2;
    VINEYARD_CHECK_OK(client1.Open(ipc_socket));
    LOG(INFO) << "Client1 connected to IPCServer: " << client1.IPCSocket();
    VINEYARD_CHECK_OK(client2.Open(ipc_socket));
    LOG(INFO) << "Client2 connected to IPCServer: " << client2.IPCSocket();

    std::vector<double> double_array = {1.0, 7.0, 3.0, 4.0, 2.0};
    ArrayBuilder<double> builder1(client1, double_array);
    auto sealed_double_array =
        std::dynamic_pointer_cast<Array<double>>(builder1.Seal(client1));

    ObjectID id = sealed_double_array->id();

    std::shared_ptr<Array<double>> array;
    auto status = client2.GetObject(id, array);
    CHECK(status.IsObjectNotExists());
    CHECK(array == nullptr);

    LOG(INFO) << "Passed session isolation test...";
    client1.Disconnect();
    client2.Disconnect();
  }

  {  // test session deletion (case 1)
    Client client1, client2;
    Client client3;
    auto session_socket_path = client1.IPCSocket();
    VINEYARD_CHECK_OK(client1.Open(ipc_socket));
    LOG(INFO) << "Client1 connected to IPCServer: " << client1.IPCSocket();

    VINEYARD_CHECK_OK(client1.Fork(client2));
    client2.CloseSession();
    client1.Disconnect();
    auto status = client3.Connect(session_socket_path);
    CHECK(status.IsConnectionFailed());
    LOG(INFO) << "Passed session deletion test 1...";
  }

  {  // test session deletion (case 2)
    std::vector<Client> clients(8);
    VINEYARD_CHECK_OK(clients[0].Open(ipc_socket));
    LOG(INFO) << "Client[0] connected to IPCServer: " << clients[0].IPCSocket();

    auto session_socket_path = clients[0].IPCSocket();
    for (size_t i = 1; i < clients.size(); ++i) {
      VINEYARD_CHECK_OK(clients[i].Connect(session_socket_path));
    }
    for (size_t i = 0; i < clients.size(); ++i) {
      clients[i].CloseSession();
    }
    Client clientx;
    auto status = clientx.Connect(session_socket_path);
    CHECK(status.IsConnectionFailed() || status.IsIOError());
    LOG(INFO) << "Passed session deletion test 2...";
  }
  return 0;
}