#pragma once

#include "HttpServlet.h"

class UploadServlet : public HttpServlet {
protected:
    void doGet(HttpServletRequest& req, HttpServletResponse& res) override;
    void doPost(HttpServletRequest& req, HttpServletResponse& res) override;
};
