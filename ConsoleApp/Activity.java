import java.io.*;

public class Activity {
    private String dirName = null;

    public static void main(String[] args)
    {
        new Activity().onCreate();
    }

    public Activity()
    {
    }

    public void onCreate()
    {
        AsyncTask uploadAsyncTask = new UploadAsyncTask().execute();
        System.out.println("Waiting for Callback");
        try
        {
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
            br.readLine();
        } catch(Exception e)
        {
            throw new RuntimeException(e);
        }
    }
}
