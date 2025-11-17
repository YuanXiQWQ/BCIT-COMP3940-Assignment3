#include "Socket.h"
#include "ServerSocket.h"
#include <stddef.h>
main() {
  ServerSocket *ss = new ServerSocket(8888);
	if (ss != NULL) {
		Socket *cs = ss->Accept();
		char *req = cs->getRequest();
		char *res = new char(50);
		cs->sendResponse(res);
	}
}
