#pragma once

#include "HttpServletRequest.hpp"
#include "HttpServletResponse.hpp"

class HttpServlet {
public:
    virtual ~HttpServlet() = default;

    virtual void doGet(const HttpServletRequest& request, HttpServletResponse& response) = 0;
    virtual void doPost(const HttpServletRequest& request, HttpServletResponse& response) = 0;
};
