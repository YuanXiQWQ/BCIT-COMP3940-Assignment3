import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

public class UploadServlet extends HttpServlet {

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
    {
        try
        {
            String html;
            File form = new File("Form.html");
            if(form.exists())
            {
                html = readFileUTF8(form);
            } else
            {
                html = "<!doctype html><html><head><meta " +
                        "charset='utf-8'><title>Upload</title></head><body>" +
                        "<h2>File Upload</h2>" + "<form method='POST' action='/' " +
                        "enctype='multipart/form-data'>" +
                        "Caption: <input type='text' name='caption'><br/><br/>" +
                        "Date: <input type='date' name='date'><br/>" +
                        "File: <input type='file' name='fileName'><br/><br/>" +
                        "<input type='submit' value='Submit'>" + "</form></body></html>";
            }
            response.getOutputStream().write(html.getBytes(StandardCharsets.UTF_8));
        } catch(Exception ex)
        {
            safeWriteError(response, ex);
        }
    }

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
    {
        try
        {
            String contentType = request.getHeader("content-type");
            if(contentType == null || !contentType.toLowerCase(Locale.ROOT)
                    .startsWith("multipart/form-data"))
            {
                throw new BadRequestException("Unsupported Content-Type: " + contentType);
            }

            String boundary = extractBoundary(contentType);
            if(boundary == null)
            {
                throw new BadRequestException("Boundary not found");
            }

            byte[] bodyBytes = readAllBytes(request.getInputStream());
            String bodyStr = new String(bodyBytes, StandardCharsets.ISO_8859_1);
            String delimiter = "--" + boundary;
            String[] rawParts = bodyStr.split(Pattern.quote(delimiter));

            String caption = "";
            String date = "";
            String originalName = null;
            byte[] filePayload = null;

            for(String part : rawParts)
            {
                if(part == null)
                {
                    continue;
                }
                String trimmed = part.trim();
                if(trimmed.isEmpty() || "--".equals(trimmed))
                {
                    continue;
                }

                int headerEnd = part.indexOf("\r\n\r\n");
                if(headerEnd < 0)
                {
                    throw new MultipartParseException("Malformed part: missing CRLFCRLF");
                }

                String headerBlock = part.substring(0, headerEnd);
                String content = part.substring(headerEnd + 4);
                if(content.endsWith("\r\n"))
                {
                    content = content.substring(0, content.length() - 2);
                }

                String disposition = null;
                String fileName = null;

                String[] headerLines = headerBlock.split("\r\n");
                for(String hl : headerLines)
                {
                    String lower = hl.toLowerCase(Locale.ROOT);
                    if(lower.startsWith("content-disposition:"))
                    {
                        disposition = hl;
                        int fnIdx = lower.indexOf("filename=");
                        if(fnIdx >= 0)
                        {
                            String after =
                                    hl.substring(fnIdx + "filename=".length()).trim();
                            if(after.startsWith("\""))
                            {
                                int end = after.indexOf("\"", 1);
                                if(end > 1)
                                {
                                    fileName = after.substring(1, end);
                                }
                            } else
                            {
                                int sp = after.indexOf(';');
                                fileName = sp > 0
                                           ? after.substring(0, sp)
                                           : after;
                            }
                        }
                    }
                }

                String fieldName = getFieldName(disposition);

                if(fileName != null && fieldName != null)
                {
                    originalName = new File(fileName).getName();
                    filePayload = content.getBytes(StandardCharsets.ISO_8859_1);
                } else if("caption".equals(fieldName))
                {
                    caption = content;
                } else if("date".equals(fieldName))
                {
                    date = content;
                }
            }

            File imagesDir = ensureImagesDir();
            String safeCaption = sanitizeForFileName(caption);
            String safeDate = sanitizeForFileName(date);
            String finalName = getFinalName(safeCaption, safeDate, originalName);

            if(filePayload != null)
            {
                File outFile = new File(imagesDir, finalName);
                try(FileOutputStream fos = new FileOutputStream(outFile))
                {
                    fos.write(filePayload);
                }
            }

            String html = buildImagesListingHtml(imagesDir);
            response.getOutputStream().write(html.getBytes(StandardCharsets.UTF_8));

        } catch(Exception ex)
        {
            safeWriteError(response, ex);
        }
    }

    private static void safeWriteError(HttpServletResponse response, Exception ex)
    {
        try
        {
            response.getOutputStream()
                    .write(("<!doctype html><meta charset='utf-8'><pre>" + ex +
                            "</pre>").getBytes(StandardCharsets.UTF_8));
        } catch(IOException ignored)
        {
        }
    }

    private static String getFieldName(String disposition)
    {
        String fieldName = null;
        if(disposition != null)
        {
            String lower = disposition.toLowerCase(Locale.ROOT);
            int nameIdx = lower.indexOf("name=");
            if(nameIdx >= 0)
            {
                String after = disposition.substring(nameIdx + "name=".length()).trim();
                if(after.startsWith("\""))
                {
                    int end = after.indexOf("\"", 1);
                    if(end > 1)
                    {
                        fieldName = after.substring(1, end);
                    }
                } else
                {
                    int sp = after.indexOf(';');
                    fieldName = sp > 0
                                ? after.substring(0, sp)
                                : after;
                }
            }
        }
        return fieldName;
    }

    private File ensureImagesDir()
    {
        File dir = new File("images");
        if(!dir.exists())
        {
            dir.mkdirs();
        }
        return dir;
    }

    private static String getFinalName(String safeCaption, String safeDate,
                                       String originalName)
    {
        String prefix = (safeCaption.isEmpty()
                         ? ""
                         : safeCaption + "_") + (safeDate.isEmpty()
                                                 ? ""
                                                 : safeDate + "_");
        return (prefix.isEmpty()
                ? ""
                : prefix) + (originalName != null
                             ? originalName
                             : "upload.bin");
    }

    private String extractBoundary(String contentType)
    {
        String[] tokens = contentType.split(";");
        for(String t : tokens)
        {
            String s = t.trim();
            if(s.toLowerCase(Locale.ROOT).startsWith("boundary="))
            {
                String b = s.substring("boundary=".length());
                if(b.startsWith("\"") && b.endsWith("\""))
                {
                    b = b.substring(1, b.length() - 1);
                }
                return b;
            }
        }
        return null;
    }

    private byte[] readAllBytes(InputStream in) throws IOException
    {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int n;
        while((n = in.read(buf)) != -1)
        {
            baos.write(buf, 0, n);
        }
        return baos.toByteArray();
    }

    private String sanitizeForFileName(String s)
    {
        if(s == null)
        {
            return "";
        }
        String t = s.trim();
        return t.replaceAll("[^A-Za-z0-9._-]+", "_");
    }

    private String readFileUTF8(File f) throws IOException
    {
        try(FileInputStream fis = new FileInputStream(f))
        {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int n;
            while((n = fis.read(buf)) != -1)
            {
                baos.write(buf, 0, n);
            }
            return baos.toString(StandardCharsets.UTF_8);
        }
    }

    private String buildImagesListingHtml(File imagesDir)
    {
        File[] files = imagesDir.listFiles(File::isFile);
        List<String> names = files == null
                             ? Collections.emptyList()
                             : Arrays.stream(files).map(File::getName)
                                     .sorted(String.CASE_INSENSITIVE_ORDER)
                                     .collect(Collectors.toList());

        StringBuilder html = new StringBuilder();
        html.append("<!doctype html><html><head><meta " +
                "charset='utf-8'><title>Images</title></head><body>");
        html.append("<h2>Images (alphabetical)</h2>");
        if(names.isEmpty())
        {
            html.append("<p>No files in images/</p>");
        } else
        {
            html.append("<ul>");
            for(String n : names)
            {
                html.append("<li>").append(n).append("</li>");
            }
            html.append("</ul>");
        }
        html.append("<hr><a href='/'>Back to form</a>");
        html.append("</body></html>");
        return html.toString();
    }
}
