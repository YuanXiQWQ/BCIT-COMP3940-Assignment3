import java.util.*;

public class HandlerFactory {
    private static final Map<String, String> MAP = new HashMap<>();

    static
    {
        MAP.put("GET", "GetRequestHandler");
        MAP.put("POST", "PostRequestHandler");
    }

    public static RequestHandler create(String method)
    {
        String key = method == null
                     ? ""
                     : method.toUpperCase(Locale.ROOT);
        String clazz = MAP.get(key);
        if(clazz == null)
        {
            throw new BadRequestException("Unsupported method: " + method);
        }
        try
        {
            Class<?> c = Class.forName(clazz);
            return (RequestHandler) c.getDeclaredConstructor().newInstance();
        } catch(Exception e)
        {
            throw new RuntimeException("Cannot instantiate handler for " + method, e);
        }
    }
}
