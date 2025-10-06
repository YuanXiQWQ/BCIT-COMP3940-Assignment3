import java.lang.reflect.*;

public class LoggingProxy implements InvocationHandler {
    private final Object target;

    private LoggingProxy(Object target) {this.target = target;}

    public static RequestHandler wrap(RequestHandler target)
    {
        return (RequestHandler) Proxy.newProxyInstance(target.getClass().getClassLoader(),
                new Class[]{RequestHandler.class}, new LoggingProxy(target));
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable
    {
        long t0 = System.currentTimeMillis();
        System.out.println(
                "[AOP] " + target.getClass().getSimpleName() + "." + method.getName() +
                        " start");
        try
        {
            return method.invoke(target, args);
        } finally
        {
            long dt = System.currentTimeMillis() - t0;
            System.out.println("[AOP] " + target.getClass().getSimpleName() + "." +
                    method.getName() + " done in " + dt + " ms");
        }
    }
}
