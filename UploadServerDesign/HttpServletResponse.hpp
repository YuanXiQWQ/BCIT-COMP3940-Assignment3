#pragma once
#include <sstream>

using namespace std;

class HttpServletResponse {
    ostringstream& os;

public:
    HttpServletResponse(ostringstream& os);

    ostringstream& getOutputStream();
};
