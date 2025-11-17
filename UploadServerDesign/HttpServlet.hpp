#pragma once
#include "HttpServletRequest.hpp"
#include "HttpServletResponse.hpp"

class HttpServlet {
public:
    virtual ~HttpServlet() = default;

protected:
    virtual void doGet(HttpServletRequest req, HttpServletResponse res) = 0;
    virtual void doPost(HttpServletRequest req, HttpServletResponse res) = 0;
};

