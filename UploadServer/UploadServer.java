import java.net.*;
import java.io.*;
import java.util.concurrent.*;

public class UploadServer {
    private static final ExecutorService POOL = Executors.newFixedThreadPool(3);

    public static void main(String[] args) throws IOException
    {
        try(ServerSocket serverSocket = new ServerSocket(8082))
        {
            System.out.println("Listening on 8082...");
            Runtime.getRuntime().addShutdownHook(new Thread(() ->
            {
                System.out.println("Shutting down thread pool...");
                POOL.shutdown();
            }));
            while(true)
            {
                Socket socket = serverSocket.accept();
                POOL.submit(new UploadServerThread(socket));
            }
        }
    }
}
