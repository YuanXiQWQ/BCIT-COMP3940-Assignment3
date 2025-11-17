#include "UploadServlet.h"

#include "MultipartParser.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

#include <cctype>

namespace{

    std::string sanitize(const std::string &value){
        std::string out;
        out.reserve(value.size());
        for(unsigned char ch: value){
            if(std::isalnum(ch)){
                out.push_back(static_cast<char>(ch));
            } else if(ch == '-' || ch == '_' || ch == '.'){
                out.push_back(static_cast<char>(ch));
            } else if(std::isspace(ch)){
                out.push_back('_');
            }
        }
        if(out.empty()){
            out = "unknown";
        }
        return out;
    }

    std::string loadFormHtml(){
        static const char *candidates[] = {
                "Form.html",
                "./Form.html",
                "../Form.html",
                "../../Form.html"
        };

        for(const char *path: candidates){
            std::ifstream in(path, std::ios::binary);
            if(in.good()){
                std::ostringstream ss;
                ss << in.rdbuf();
                return ss.str();
            }
        }

        return "<html><body><h1>Upload</h1><form method=\"post\" enctype=\"multipart/form-data\">"
               "Caption: <input type=\"text\" name=\"caption\"/><br/>"
               "Date: <input type=\"date\" name=\"date\"/><br/>"
               "File: <input type=\"file\" name=\"fileName\"/><br/>"
               "<input type=\"submit\" value=\"Upload\"/></form></body></html>";
    }

    void writeError(HttpServletResponse &res, int status, const std::string &message){
        res.setStatus(status, message);
        res.setContentType("text/plain; charset=utf-8");
        res.write(message + "\n");
    }

} // namespace

void UploadServlet::doGet(HttpServletRequest &req, HttpServletResponse &res){
    (void) req;
    res.setStatus(200, "OK");
    res.setContentType("text/html; charset=utf-8");
    res.write(loadFormHtml());
}

void UploadServlet::doPost(HttpServletRequest &req, HttpServletResponse &res){
    if(! MultipartParser::parse(req)){
        writeError(res, 400, "Unable to parse multipart/form-data request");
        return;
    }

    if(! req.hasUploadedFile()){
        writeError(res, 400, "No uploaded file found");
        return;
    }

    std::string caption = req.getCaption();
    std::string date = req.getDate();
    if(caption.empty() || date.empty()){
        writeError(res, 400, "Missing caption or date");
        return;
    }

    const UploadedFile &upload = req.getUploadedFile();
    std::filesystem::path targetDir = std::filesystem::path("uploads");
    std::error_code ec;
    if(! std::filesystem::exists(targetDir)){
        std::filesystem::create_directories(targetDir, ec);
        if(ec){
            writeError(res, 500, "Failed to create uploads directory");
            return;
        }
    }

    std::string safeCaption = sanitize(caption);
    std::string safeDate = sanitize(date);
    std::string safeName = sanitize(upload.originalFileName);
    std::filesystem::path filePath =
            targetDir / (safeDate + "_" + safeCaption + "_" + safeName);

    std::ofstream out(filePath, std::ios::binary);
    if(! out.good()){
        writeError(res, 500, "Failed to save uploaded file");
        return;
    }
    out.write(upload.data.data(), static_cast<std::streamsize>(upload.data.size()));
    out.close();

    res.setStatus(200, "OK");
    res.setContentType("text/html; charset=utf-8");
    std::ostringstream body;
    body << "<html><body><h2>Upload successful</h2>"
         << "<p>File saved to: " << filePath.generic_string() << "</p>"
         << "<p>Caption: " << caption << "</p>"
         << "<p>Date: " << date << "</p>"
         << "<p>Original filename: " << upload.originalFileName << "</p>"
         << "</body></html>";
    res.write(body.str());
}
