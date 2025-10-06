public class ThreadSafety implements Runnable {
    int shared = 0;
    private boolean t2Completed = false;

    public static void main(String[] args)
    {
        ThreadSafety ts = new ThreadSafety();
        Thread t1 = new Thread(ts, "T1");
        t1.start();
        Thread t2 = new Thread(ts, "T2");
        t2.start();
    }

    public void run()
    {
        synchronized(this)
        {
            // T1 waits for T2 to complete first
            if(Thread.currentThread().getName().equals("T1"))
            {
                while(!t2Completed)
                {
                    try
                    {
                        this.wait();
                    } catch(InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }

            int copy = shared;
            try
            {
                Thread.sleep((int) (Math.random() * 10000));
            } catch(InterruptedException e)
            {
                Thread.currentThread().interrupt();
            }
            shared = copy + 1;
            System.out.println(Thread.currentThread().getName() + ": " + shared);

            // After T2 finishes, signal T1 to proceed
            if(Thread.currentThread().getName().equals("T2"))
            {
                t2Completed = true;
                this.notifyAll();
            }
        }
    }
}