#pragma once

#include "HttpServletRequest.h"
#include "HttpServletResponse.h"

class HttpServlet{
public:
    virtual ~HttpServlet() = default;

    void service(HttpServletRequest &req, HttpServletResponse &res);

protected:
    virtual void doGet(HttpServletRequest &req, HttpServletResponse &res);

    virtual void doPost(HttpServletRequest &req, HttpServletResponse &res);
};
