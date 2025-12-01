#pragma once

#include "HttpServlet.hpp"
#include "HttpServletRequest.hpp"
#include "HttpServletResponse.hpp"

class UploadServlet : public HttpServlet {
public:
    void doGet(const HttpServletRequest& request, HttpServletResponse& response) override;
    void doPost(const HttpServletRequest& request, HttpServletResponse& response) override;
};
