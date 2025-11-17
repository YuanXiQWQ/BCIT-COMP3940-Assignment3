#include "HttpServlet.h"

#include <stdexcept>

void HttpServlet::service(HttpServletRequest &req, HttpServletResponse &res){
    const std::string &method = req.getMethod();
    if(method == "GET"){
        doGet(req, res);
    } else if(method == "POST"){
        doPost(req, res);
    } else{
        res.setStatus(405, "Method Not Allowed");
        res.setHeader("Allow", "GET, POST");
        res.write("Method not allowed\n");
    }
}

void HttpServlet::doGet(HttpServletRequest &, HttpServletResponse &){
    throw std::runtime_error("GET not supported");
}

void HttpServlet::doPost(HttpServletRequest &, HttpServletResponse &){
    throw std::runtime_error("POST not supported");
}
