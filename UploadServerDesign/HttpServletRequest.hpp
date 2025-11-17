#pragma once
#include <sstream>

using namespace std;

class HttpServletRequest {
  stringstream& is;
  string caption;
  string date;
  string filename;

public:
  HttpServletRequest(stringstream& is);

  stringstream& getInputStream();
  string getCaption();
  string getDate();
  string getFilename();

  void setCaption(string caption);
  void setDate(string date);
  void setFilename(string filename);
};
