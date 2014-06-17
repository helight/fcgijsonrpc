/*
 * jsonrpc.cc
 *
 *  Created on: 2010-11-25
 *      Author: laelli,helightxu
 * add get method
 * add post for fastcgi
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "jsonrpc/jsonrpc.h"
#include "fcgi_stdio.h"
#include "fcgi_config.h"

class CgiInput
{
public:
    /*!
     * \brief Query the value of an environment variable
     *
     * In the default implementation, this is a wrapper for std::getenv()
     * \param name The name of an environment variable
     * \return The value of the requested environment variable, or an empty
     * string if not found.
     */
    std::string GetEnv(const char* name) const
    {
        char *var = std::getenv(name);
        return (0 == var) ? std::string("") : var;
    }

    size_t Read(char *data, size_t length)
    {
        FCGI_gets(data);
        return strlen(data);
        std::cin.read(data, length);
        return std::cin.gcount();
    }
};

bool StringsAreEqual(const std::string& s1, const std::string& s2)
{
    std::string::const_iterator p1 = s1.begin();
    std::string::const_iterator p2 = s2.begin();
    std::string::const_iterator l1 = s1.end();
    std::string::const_iterator l2 = s2.end();

    while(p1 != l1 && p2 != l2)
    {
        if(std::toupper(*(p1++)) != std::toupper(*(p2++)))
            return false;
    }
    return (s2.size() == s1.size()) ? true : false;
}

std::string& replace_str(std::string& str
                         , const std::string& old_value
                         , const std::string& new_value)
{
    while(true)   {
        std::string::size_type   pos(0);
        if(   (pos=str.find(old_value))!=std::string::npos   )
            str.replace(pos,old_value.length(),new_value);
        else break;
    }
    return str;
}

class CgiEnv
{
public:
    CgiEnv()
    {
        CgiInput input;
        ReadEnvironmentVariables(&input);

        if(StringsAreEqual(RequestMethod, "post") || StringsAreEqual(RequestMethod, "put"))
        {
            // Don't use auto_ptr, but vector instead
            std::vector<char> data(ContentLength);

            // If input is 0, use the default implementation of CgiInput
            if (ContentLength)
            {
                if (input.Read(&data[0], ContentLength ) != ContentLength )
                    throw std::runtime_error("I/O error");
                PostData = std::string(&data[0], ContentLength);
		//printf("%s------%d\n", PostData.c_str(), 10);
            }
        }
        else if (StringsAreEqual(RequestMethod, "get"))
        {
            // Don't use auto_ptr, but vector instead
			//printf("%s\n", QueryString.c_str());
            PostData = replace_str(QueryString, "%22","\"");
        }
        else if (RequestMethod.empty())
        {
            char ch;
            // If input is 0, use the default implementation of CgiInput
            while ((ch = getc(stdin)) != '\n')
            {
                PostData.push_back(ch);
            }

            ContentLength = PostData.size();
            RequestMethod = "POST";
        }
    }

public:
    unsigned long ServerPort;
    unsigned long ContentLength;
    bool UsingHTTPS;
    std::string ServerSoftware;
    std::string ServerName;
    std::string GatewayInterface;
    std::string ServerProtocol;
    std::string RequestMethod;
    std::string PathInfo;
    std::string PathTranslated;
    std::string ScriptName;
    std::string QueryString;
    std::string RemoteHost;
    std::string RemoteAddr;
    std::string AuthType;
    std::string RemoteUser;
    std::string RemoteIdent;
    std::string ContentType;
    std::string Accept;
    std::string UserAgent;
    std::string PostData;
    std::string RedirectRequest;
    std::string RedirectURL;
    std::string RedirectStatus;
    std::string Referrer;
    std::string Cookie;

private:
    void ReadEnvironmentVariables(const CgiInput* input)
    {
        ServerSoftware = input->GetEnv("SERVER_SOFTWARE");
        ServerName = input->GetEnv("SERVER_NAME");
        GatewayInterface = input->GetEnv("GATEWAY_INTERFACE");
        ServerProtocol = input->GetEnv("SERVER_PROTOCOL");

        ServerPort = std::atol(input->GetEnv("SERVER_PORT").c_str());

        RequestMethod = input->GetEnv("REQUEST_METHOD");
        PathInfo = input->GetEnv("PATH_INFO");
        PathTranslated = input->GetEnv("PATH_TRANSLATED");
        ScriptName = input->GetEnv("SCRIPT_NAME");
        QueryString = input->GetEnv("QUERY_STRING");
        RemoteHost = input->GetEnv("REMOTE_HOST");
        RemoteAddr = input->GetEnv("REMOTE_ADDR");
        AuthType = input->GetEnv("AUTH_TYPE");
        RemoteUser = input->GetEnv("REMOTE_USER");
        RemoteIdent = input->GetEnv("REMOTE_IDENT");
        ContentType = input->GetEnv("CONTENT_TYPE");

        ContentLength = std::atol(input->GetEnv("CONTENT_LENGTH").c_str());

        Accept = input->GetEnv("HTTP_ACCEPT");
        UserAgent = input->GetEnv("HTTP_USER_AGENT");
        RedirectRequest = input->GetEnv("REDIRECT_REQUEST");
        RedirectURL = input->GetEnv("REDIRECT_URL");
        RedirectStatus = input->GetEnv("REDIRECT_STATUS");
        Referrer = input->GetEnv("HTTP_REFERER");
        Cookie = input->GetEnv("HTTP_COOKIE");

        UsingHTTPS =  StringsAreEqual("on", input->GetEnv("HTTPS"));
    }
};

namespace json {
namespace rpc {
void Container::Run(json::rpc::Handler& handler, json::Value &resposne)
{
    CgiEnv env;

    handler.Process(env.PostData, resposne, NULL);
	resposne["reqdata"] = env.PostData;
    // printf("%s\n", handler.GetString(resposne).c_str());
}
} // namespace rpc
} // namespace json
