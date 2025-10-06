public class UploadAsyncTask extends AsyncTask {
    @Override
    protected void onPreExecute()
    {
    }

    @Override
    protected String doInBackground()
    {
        return new UploadClient().uploadFile();
    }

    @Override
    protected void onPostExecute(String result)
    {
        System.out.println(result);
    }
}
