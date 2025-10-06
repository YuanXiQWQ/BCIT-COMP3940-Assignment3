import java.net.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class UploadServerThread extends Thread {
    private final Socket socket;

    public UploadServerThread(Socket socket)
    {
        super("DirServerThread");
        this.socket = socket;
    }

    @Override
    public void run()
    {
        try
        {
            socket.setSoTimeout(15000);
            InputStream in = socket.getInputStream();

            ByteArrayOutputStream headerBuf = new ByteArrayOutputStream();
            int state = 0;
            while(true)
            {
                int b = in.read();
                if(b == -1)
                {
                    break;
                }
                headerBuf.write(b);
                if(state == 0 && b == '\r')
                {
                    state = 1;
                } else if(state == 1 && b == '\n')
                {
                    state = 2;
                } else if(state == 2 && b == '\r')
                {
                    state = 3;
                } else if(state == 3 && b == '\n')
                {
                    break;
                } else
                {
                    state = 0;
                }
            }

            String headerText =
                    headerBuf.toString(java.nio.charset.StandardCharsets.ISO_8859_1);
            String[] headerLines = headerText.split("\r\n");
            if(headerLines.length == 0)
            {
                socket.close();
                return;
            }

            String requestLine = headerLines[0];
            String[] parts = requestLine.split(" ");
            String method = parts.length > 0
                            ? parts[0]
                            : "";
            String uri = parts.length > 1
                         ? parts[1]
                         : "/";

            java.util.Map<String, String> headers = new java.util.LinkedHashMap<>();
            for(int i = 1; i < headerLines.length; i++)
            {
                String line = headerLines[i];
                if(line.isEmpty())
                {
                    break;
                }
                int idx = line.indexOf(':');
                if(idx > 0)
                {
                    String k = line.substring(0, idx).trim();
                    String v = line.substring(idx + 1).trim();
                    headers.put(k, v);
                }
            }

            int contentLength = 0;
            String cl =
                    headers.getOrDefault("Content-Length", headers.get("content-length"));
            if(cl != null)
            {
                try
                {
                    contentLength = Integer.parseInt(cl);
                } catch(NumberFormatException ignored)
                {
                }
            }
            byte[] body = new byte[contentLength];
            int off = 0;
            while(off < contentLength)
            {
                int n = in.read(body, off, contentLength - off);
                if(n < 0)
                {
                    break;
                }
                off += n;
            }
            java.io.ByteArrayInputStream bodyIn = new java.io.ByteArrayInputStream(body);

            java.io.ByteArrayOutputStream bodyOut = new java.io.ByteArrayOutputStream();
            HttpServletRequest req = new HttpServletRequest(method, uri, headers, bodyIn);
            HttpServletResponse res = new HttpServletResponse(bodyOut);

            HttpServlet httpServlet = new UploadServlet();

            String status;
            byte[] responseBody;
            try
            {
                RequestHandler handler = LoggingProxy.wrap(HandlerFactory.create(method));
                handler.handle(httpServlet, req, res);

                responseBody = bodyOut.toByteArray();
                status = "HTTP/1.1 200 OK\r\n";
            } catch(BadRequestException | MultipartParseException e)
            {
                String html = "<!doctype html><meta charset='utf-8'><title>400</title>" +
                        "<body><h3>Bad Request</h3><pre>" + e.getMessage() +
                        "</pre></body>";
                responseBody = html.getBytes(StandardCharsets.UTF_8);
                status = "HTTP/1.1 400 Bad Request\r\n";
            } catch(Exception e)
            {
                String html = "<!doctype html><meta charset='utf-8'><title>500</title>" +
                        "<body><h3>Internal Server Error</h3><pre>" + e + "</pre></body>";
                responseBody = html.getBytes(StandardCharsets.UTF_8);
                status = "HTTP/1.1 500 Internal Server Error\r\n";
            }

            String respHeaders =
                    "Content-Type: text/html; charset=UTF-8\r\n" + "Content-Length: " +
                            responseBody.length + "\r\n" + "Connection: close\r\n\r\n";

            OutputStream out = socket.getOutputStream();
            out.write(status.getBytes(java.nio.charset.StandardCharsets.ISO_8859_1));
            out.write(respHeaders.getBytes(java.nio.charset.StandardCharsets.ISO_8859_1));
            out.write(responseBody);
            out.flush();
            socket.close();
        } catch(Exception e)
        {
            e.printStackTrace();
        }
    }
}
