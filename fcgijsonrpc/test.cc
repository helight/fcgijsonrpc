#include <iostream>
#include <jsonrpc/jsonrpc.h>
#include "fcgi_config.h"
#include "fcgi_stdio.h"

class TestRpc
{
public:
    TestRpc(json::rpc::Handler& handler)
    {
        handler.AddMethod(new json::rpc::RpcMethod<TestRpc>(*this,
                                                            &TestRpc::Print,
                                                            "Test.Print"));
        handler.AddMethod(new json::rpc::RpcMethod<TestRpc>(*this,
                                                            &TestRpc::show_project,
                                                            "show_project"));
    }

    bool Print(const json::Value& msg, json::Value& response, void* data)
    {
        response["jsonrpc"] = "2.0";
        response["id"] = msg["id"];
        response["result"] = "success";

        return true;
    }
    bool show_project(const json::Value& msg, json::Value& response, void* data)
    {
        const json::Value& project_id = msg["project_id"];
        response["jsonrpc"] = "2.0";
        response["id"] = project_id;
        response["result"] = "success";

        return true;
    }
};

int main(void)
{
    json::rpc::Handler handler;
    TestRpc rpc1(handler);
	int count = 0;
	while (FCGI_Accept() >= 0) {
		json::Value resposne;
        json::rpc::Container container;
		printf("Content-type:text/html;charset=utf-8\r\n\r\n");
        container.Run(handler, resposne);
        printf("%s\n", handler.GetString(resposne).c_str());
        printf("<br><h1> %d</h1>", count++);
		FCGI_Finish();
	}
}
