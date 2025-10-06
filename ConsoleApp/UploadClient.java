import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.time.LocalDate;
import java.util.*;

public class UploadClient {
    private static final String SERVER_HOST = "localhost";
    private static final int SERVER_PORT = 8082;

    public static void main(String[] args)
    {
        try
        {
            String filePath = args.length > 0
                              ? args[0]
                              : "AndroidLogo.png";
            String caption = args.length > 1
                             ? args[1]
                             : "console";
            String date = args.length > 2
                          ? args[2]
                          : LocalDate.now().toString();
            String html = new UploadClient().uploadFile(filePath, caption, date);
            System.out.println(html);
        } catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    public String uploadFile()
    {
        return uploadFile("AndroidLogo.png", "console", LocalDate.now().toString());
    }

    public String uploadFile(String filePath, String caption, String date)
    {
        try(Socket socket = new Socket(SERVER_HOST, SERVER_PORT))
        {
            socket.setSoTimeout(15000);

            String boundary = "----Boundary" + System.currentTimeMillis();
            ByteArrayOutputStream body = new ByteArrayOutputStream();

            writeString(body, "--" + boundary + "\r\n");
            writeString(body, "Content-Disposition: form-data; name=\"caption\"\r\n\r\n");
            writeString(body, caption + "\r\n");

            writeString(body, "--" + boundary + "\r\n");
            writeString(body, "Content-Disposition: form-data; name=\"date\"\r\n\r\n");
            writeString(body, date + "\r\n");

            File file = new File(filePath);
            if(!file.exists() || !file.isFile())
            {
                return "File not found: " + file.getAbsolutePath();
            }

            writeString(body, "--" + boundary + "\r\n");
            writeString(body,
                    "Content-Disposition: form-data; name=\"fileName\"; filename=\"" +
                            file.getName() + "\"\r\n");
            writeString(body, "Content-Type: application/octet-stream\r\n\r\n");
            try(FileInputStream fis = new FileInputStream(file))
            {
                byte[] buf = new byte[8192];
                int n;
                while((n = fis.read(buf)) != -1)
                {
                    body.write(buf, 0, n);
                }
            }
            writeString(body, "\r\n");
            writeString(body, "--" + boundary + "--\r\n");

            byte[] bodyBytes = body.toByteArray();
            String headers =
                    "POST / HTTP/1.1\r\n" + "Host: " + SERVER_HOST + ":" + SERVER_PORT +
                            "\r\n" + "Connection: close\r\n" +
                            "Content-Type: multipart/form-data; boundary=" + boundary +
                            "\r\n" + "Content-Length: " + bodyBytes.length + "\r\n\r\n";

            OutputStream out = socket.getOutputStream();
            out.write(headers.getBytes(StandardCharsets.ISO_8859_1));
            out.write(bodyBytes);
            out.flush();

            return readHttpResponse(socket.getInputStream());
        } catch(Exception e)
        {
            return "Error: " + e;
        }
    }

    private static void writeString(OutputStream out, String s) throws IOException
    {
        out.write(s.getBytes(StandardCharsets.ISO_8859_1));
    }

    private static String readHttpResponse(InputStream in) throws IOException
    {
        ByteArrayOutputStream headerBuf = new ByteArrayOutputStream();
        int c;
        while((c = in.read()) != -1)
        {
            headerBuf.write(c);
            byte[] b = headerBuf.toByteArray();
            int n = b.length;
            if(n >= 4 && b[n - 4] == '\r' && b[n - 3] == '\n' && b[n - 2] == '\r' &&
                    b[n - 1] == '\n')
            {
                break;
            }
        }

        int contentLength = getContentLength(headerBuf);

        ByteArrayOutputStream body = new ByteArrayOutputStream();
        if(contentLength >= 0)
        {
            byte[] buf = new byte[8192];
            int remaining = contentLength;
            while(remaining > 0)
            {
                int n = in.read(buf, 0, Math.min(buf.length, remaining));
                if(n < 0)
                {
                    break;
                }
                body.write(buf, 0, n);
                remaining -= n;
            }
        } else
        {
            byte[] buf = new byte[8192];
            int n;
            while((n = in.read(buf)) != -1)
            {
                body.write(buf, 0, n);
            }
        }
        return body.toString(StandardCharsets.UTF_8);
    }

    private static int getContentLength(ByteArrayOutputStream headerBuf)
            throws IOException
    {
        String headerText = headerBuf.toString(StandardCharsets.ISO_8859_1);
        Map<String, String> headers = getStringStringMap(headerText);

        int contentLength = -1;
        if(headers.containsKey("content-length"))
        {
            try
            {
                contentLength = Integer.parseInt(headers.get("content-length"));
            } catch(NumberFormatException ignored)
            {
            }
        }
        return contentLength;
    }

    private static Map<String, String> getStringStringMap(String headerText)
            throws IOException
    {
        Map<String, String> headers = new LinkedHashMap<>();
        BufferedReader br = new BufferedReader(new StringReader(headerText));
        br.readLine();
        String line;
        while((line = br.readLine()) != null && !line.isEmpty())
        {
            int idx = line.indexOf(':');
            if(idx > 0)
            {
                headers.put(line.substring(0, idx).trim().toLowerCase(Locale.ROOT),
                        line.substring(idx + 1).trim());
            }
        }
        return headers;
    }
}
