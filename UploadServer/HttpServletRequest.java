import java.io.*;
import java.util.*;

public class HttpServletRequest {
    private final String method;
    private final String uri;
    private final Map<String, String> headers;
    private final InputStream inputStream;

    public HttpServletRequest(String method, String uri, Map<String, String> headers,
                              InputStream inputStream)
    {
        this.method = method;
        this.uri = uri;
        this.headers = new HashMap<>();
        if(headers != null)
        {
            for(Map.Entry<String, String> e : headers.entrySet())
            {
                this.headers.put(e.getKey().toLowerCase(Locale.ROOT), e.getValue());
            }
        }
        this.inputStream = inputStream;
    }

    public String getMethod() {return method;}

    public String getRequestURI() {return uri;}

    public String getHeader(String name)
    {
        if(name == null)
        {
            return null;
        }
        return headers.get(name.toLowerCase(Locale.ROOT));
    }

    public Map<String, String> getHeaders() {return Collections.unmodifiableMap(headers);}

    public InputStream getInputStream() {return inputStream;}
}
