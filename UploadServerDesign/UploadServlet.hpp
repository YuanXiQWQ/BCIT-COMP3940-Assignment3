#pragma once
#include "HttpServlet.hpp"

class UploadServlet : public HttpServlet {
public:
    void doGet(HttpServletRequest req, HttpServletResponse res) override;
    void doPost(HttpServletRequest req, HttpServletResponse res) override;
};
