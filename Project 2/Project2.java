import java.util.concurrent.Semaphore;
import java.util.Queue;
import java.util.LinkedList;

public class Project2 extends Thread
{

    // pre-defined variables
    final static int MAX_CUSTOMERS = 20;
    final static int INFODESKS = 1;
    final static int ANNOUNCERS = 1;
    final static int AGENTS = 2;
    final static int MAX_QUEUE = 4;
    public static int TOTAL_CUSTOMERS = 0;
    //public static Queue<Integer> numberQueue = new LinkedList<Integer>();
    public static Queue<Integer> numberQueue = new LinkedList<Integer>();
    public static Queue<Integer> agentQueue = new LinkedList<Integer>();

    // semaphores
    Semaphore customerReady = new Semaphore(0);
    Semaphore customerToBeServed = new Semaphore(0);
    Semaphore agentFree = new Semaphore(AGENTS);
    Semaphore mutex1 = new Semaphore(1, true);
    Semaphore mutex2 = new Semaphore(1, true);
    Semaphore mutex3 = new Semaphore(1, true);

    

    class CustomerThread extends Thread
    {
        private int customerNumber;
        private int waitNumber;
        
        public CustomerThread (int c)
        {
            customerNumber = c;
            System.out.println("Customer " + customerNumber + " created");
        }

        public void run()
        {
            try
            {
                mutex1.acquire();
                waitNumber = numberQueue.remove();
                mutex1.release();

                System.out.println("Customer " + customerNumber + " gets number " + waitNumber + ", enters waiting room");
                customerReady.release();
            }
            catch(Exception e)
            {
                System.out.println("ERROR in Customer");
            }
            
        }
    }

    class InfodeskThread extends Thread
    {
        private int count;

        public InfodeskThread(int c)
        {
            count = c;
            System.out.println("Information desk created");
        }

        public void run()
        {
            try
            {
                for (int i = 1; i <= count; i++)
                    numberQueue.add(i);
            }
            catch(Exception e)
            {
                System.out.println("ERROR in Infodesk");
            }
        }
    }

    class AnnouncerThread extends Thread
    {
        private int count = 1;

        AnnouncerThread()
        {
            System.out.println("Accouncer created");
        }

        public void run()
        {
            try
            {
                while (true)
                {
                    customerReady.acquire();
                    agentFree.acquire();
                    mutex2.acquire();
                    mutex3.acquire();

                    System.out.println("Accouncer calls number " + count);
                    agentQueue.add(count);
                    System.out.println("Customer " + count + " moves to the agent line");
                    
                    mutex2.release();
                    mutex3.release();

                    customerToBeServed.release();
                    count++;
                }
            }
            catch (Exception e)
            {
                System.out.println("ERROR in Announcer");
            }
        }
    }

    class AgentThread extends Thread
    {
        private int currentAgent;
        private int currentCustomer;

        public AgentThread(int a)
        {
            currentAgent = a;
            System.out.println("Agent " + currentAgent + " created");

        }

        public void run()
        {

            try
            {
                while (true)
                {
                    customerToBeServed.acquire();
                    mutex3.acquire();
                    currentCustomer = numberQueue.remove();
                    System.out.println("Agent " + currentAgent + " is serving customer " + currentCustomer);
                    mutex3.release();

                    System.out.println("Agent " + currentAgent + " asks customer " + currentCustomer + " to take photo and eye exam");
                    System.out.println("Customer " + currentCustomer + " completes photo and eye exam for agent " + currentAgent);
                    System.out.println("Agent " + currentAgent + " gives license to customer " + currentCustomer);
                    System.out.println("Customer " + currentCustomer + " gets license and departs");
   
                    TOTAL_CUSTOMERS++;
                }
            }
            catch (Exception e)
            {
                System.out.println("ERROR in Agent");
            }
        }
    }

    public static void main (String[] args)
    {
        Project2 a = new Project2();
        Thread[] customer = new Thread[MAX_CUSTOMERS];
        Thread[] infodesk = new Thread[INFODESKS];
        Thread[] announcer = new Thread[ANNOUNCERS];
        Thread[] agent = new Thread[AGENTS];

        // Run Infodesk threads
        infodesk[0] = a.new InfodeskThread(MAX_CUSTOMERS);
        infodesk[0].start();

        // Run Announcer thread
        announcer[0] = a.new AnnouncerThread();
        announcer[0].start();

        // Run Agent thread
        for (int i = 0; i < AGENTS; i++)
        {
            agent[i] = a.new AgentThread(i);
            agent[i].start();
        }

        // Run Customer thread after other threads, in case of error
        for (int i = 0; i < MAX_CUSTOMERS; i++)
        {
            try
            {
                customer[i] = a.new CustomerThread(i);
                customer[i].start();
            }
            catch(Exception e)
            {
                System.out.println("Error when joining custumer at number " + i);
            }
        }

        // Join Customer thread
        for (int i = 0; i < MAX_CUSTOMERS; i++)
        {
            try
            {
                customer[i].join();
                System.out.println("Customer " + i + " was joined");
            }
            catch(Exception e)
            {
                System.out.println("Error when joining custumer at number " + i);
            }
        }

        // Join Agent thread
        for (int i = 0; i < AGENTS; i++)
        {
            try
            {
                agent[i].interrupt();
                agent[i].join(1);
            }
            catch(Exception e)
            {
                System.out.println("Error when creating agent at number " + i);
            }
        }
        System.exit(0);
    }
}