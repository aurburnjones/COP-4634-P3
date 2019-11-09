/**
 * This multi-threaded program uses mutex locks and semaphores. It creates 20
 * "lizard" threads and 2 "cat" threads. Each lizard thread will sleep for some 
 * random amount of time and wake up hungry. The hungry lizard thread will attempt 
 * to cross the driveway in a safe manner. Crossing the driveway will take some 
 * number of seconds. Once on the other side of the driveway, the lizard will eat 
 * for some random amount of time in the monkey grass. After eating, the lizard will 
 * return home to the sago palm as soon as it safely can and sleep again. Each of 
 * the two cat threads will periodically sleep but when awake they will check out the 
 * driveway to see if there are any lizards to play with. If the cat sees too many 
 * lizards, it will play with them, causing the entire program to terminate. The two
 * cat threads will not differ in their behavior but they will differ in which is
 * awake to check on the lizards and which sleeps.
 *
 * @author Aurburn Jones
 * @author Derick Masters
 * @date 11/9/2019
 * @info Course COP 4634
 * @bug No known bugs.
 */
 
/**
 * Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>          // DM AJ

/*
 * Function prototypes
 */
void * lizardThread( void * param );
void * catThread( void * param );

/*
 * Make this 1 to check for lizards traveling in both directions
 * Leave it 0 to allow bidirectional travel
 */
#define UNIDIRECTIONAL       0

/*
 * Set this to the number of seconds you want the lizard world to
 * be simulated.  
 * Try 30 for development and 120 for more thorough testing.
 */
#define WORLDEND             30

/*
 * Number of lizard threads to create
 */
#define NUM_LIZARDS          10

/*
 * Number of cat threads to create
 */
#define NUM_CATS             2

/*	
 * Maximum lizards crossing at once before alerting cats
 */
#define MAX_LIZARD_CROSSING  4

/*
 * Maximum seconds for a lizard to sleep
 */
#define MAX_LIZARD_SLEEP     3

/*
 * Maximum seconds for a cat to sleep
 */
#define MAX_CAT_SLEEP        3

/*
 * Maximum seconds for a lizard to eat
 */
#define MAX_LIZARD_EAT       5

/*
 * Number of seconds it takes to cross the driveway
 */
#define CROSS_SECONDS        2


/*
 * Global variables
 */
int numCrossingSago2MonkeyGrass;
int numCrossingMonkeyGrass2Sago;
int debug;
int running;
int numOfLizardsCrossing=0;                     // DM AJ
sem_t sem_liz;                                  // DM AJ
pthread_mutex_t sago2grass;                     // DM AJ
pthread_mutex_t grass2sago;                     // DM AJ
pthread_mutex_t numLizardsCrossing;             // DM AJ

/*
 * main()
 *
 * Initializes variables, locks, & semaphores. Checks for debug flag.
 * Starts the cat and the lizard threads. Blocks until all threads
 * have terminated.
 */
int main(int argc, char **argv)
{
	/*
	* Local variables
	*/
	pthread_t tid_liz[NUM_LIZARDS], tid_cat[NUM_CATS];              // DM AJ
	intptr_t i;                                                     // DM AJ

	/*
	* Checks for the debugging flag (-d)
	*/
	debug = 0;
	if (argc > 1)
		if (strncmp(argv[1], "-d", 2) == 0)
		debug = 1;

	/*
	* Initializes variables
	*/
	numCrossingSago2MonkeyGrass = 0;
	numCrossingMonkeyGrass2Sago = 0;
	running = 1;

	/*
	* Initializes random number generator
	*/
	srandom( (unsigned int)time(NULL) );

	/*
	* Initializes locks and/or semaphores
	*/
	sem_init(&sem_liz, 0, MAX_LIZARD_CROSSING);                     // DM AJ
	pthread_mutex_init(&sago2grass, NULL);                          // DM AJ
	pthread_mutex_init(&grass2sago, NULL);                          // DM AJ
	pthread_mutex_init(&numLizardsCrossing, NULL);                  // DM AJ
	
	/*
	* Creates NUM_LIZARDS lizard threads
	*/
	for (i=0; i<NUM_LIZARDS; i++)                                                   // DM AJ
		pthread_create(&tid_liz[i], NULL, &lizardThread, (void *)i);            // DM AJ

	/*
	* Creates NUM_CATS cat threads
	*/
	for (i=0; i<NUM_CATS; i++)                                                      // DM AJ
		pthread_create(&tid_cat[i], NULL, &catThread, (void *)i);               // DM AJ

	/*
	* Lets the world run for a while
	*/
	sleep( WORLDEND );

	/*
	* That's it - the end of the world
	*/
	running = 0;

	/*
	* Waits until all threads terminate
	*/
	for (i=0; i<NUM_LIZARDS; i++)                                   // DM AJ
		pthread_join(tid_liz[i], NULL);                         // DM AJ
	for (i=0; i<NUM_CATS; i++)                                      // DM AJ
		pthread_join(tid_cat[i], NULL);                         // DM AJ

	/*
	* Deletes the locks and semaphores
    */
	pthread_mutex_destroy(&sago2grass);                             // DM AJ
	pthread_mutex_destroy(&grass2sago);                             // DM AJ
	pthread_mutex_destroy(&numLizardsCrossing);                     // DM AJ
	sem_destroy(&sem_liz);                                          // DM AJ

	/*
	* Exits happily
	*/
	return 0;
}


/*
 * Function prototypes
 */
void lizard_sleep(int num);
void cat_sleep(int num);
void sago_2_monkeyGrass_is_safe(int num);
void cross_sago_2_monkeyGrass(int num);
void made_it_2_monkeyGrass(int num);
void lizard_eat(int num);
void monkeyGrass_2_sago_is_safe(int num);
void cross_monkeyGrass_2_sago(int num);
void made_it_2_sago(int num);


/*
 * lizardThread()
 *
 * Simulates lizards crossing back and forth between a sago palm 
 * and some monkey grass.  
 * input: lizard number
 */
void * lizardThread( void * param )
{
	int num = (intptr_t)param;              // DM AJ

	if (debug)
    {
		printf("[%2d] lizard is alive\n", num);
		fflush(stdout);
    }

	while(running)
    {
		/* 
		* Puts lizard to sleep.
		*/
		lizard_sleep(num);                                              // DM AJ
		/*
		* Checks if it is safe to cross from sago to monkey grass.
		*/
		sago_2_monkeyGrass_is_safe(num);                                // DM AJ
		/*
		* Lizard crosses from sago to monkey grass.
		*/
		cross_sago_2_monkeyGrass(num);                                  // DM AJ
		/*
		* Lets other lizards know it made it across to monkey grass.
		*/
		made_it_2_monkeyGrass(num);                                     // DM AJ
		/*
		* Lizard eats monkey grass for some time.
		*/
		lizard_eat(num);                                                // DM AJ
		/*
		* Checks if it is safe to cross from monkey grass to sago.
		*/
		monkeyGrass_2_sago_is_safe(num);                                // DM AJ
		/*
		* Lizard crosses from monkey grass to sago.
		*/
		cross_monkeyGrass_2_sago(num);                                  // DM AJ
		/*
		* Lets other lizards know it made it across to sago.
		*/
		made_it_2_sago(num);                                            // DM AJ
    }

	pthread_exit(NULL);
}

/*
 * catThread()
 *
 * Simulates a cat that is sleeping and occasionally checking on
 * the driveway for lizards.
 * 
 * input: cat number
 */
void * catThread( void * param )
{
	int num = (intptr_t)param;              // DM AJ

	if (debug)
    {
		printf("[%2d] cat is alive\n", num);
		fflush(stdout);
    }

	while(running)
    {
		cat_sleep(num);
		/*
		* Prints the number of lizards crossing after cat wakes up if debug argument is used
		*/
		if (debug)											// DM AJ
		{	
			printf( "----Number of lizards crossing = %d\n", numOfLizardsCrossing );                // DM AJ
			fflush(stdout);                                                                         // DM AJ
		}
		/*
		* Checks for too many lizards crossing
		*/
		if (numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago > MAX_LIZARD_CROSSING)
		{
			printf( "\tThe cats are happy - they have toys.\n" );
			exit( -1 );
		}
    }
	pthread_exit(NULL);
}


/*
 * lizard_sleep()
 *
 * Simulates a lizard sleeping for a random amount of time.
 * input: lizard number
 */
void lizard_sleep(int num)
{
	int sleepSeconds;

	sleepSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_LIZARD_SLEEP);

	if (debug)
    {
		printf( "[%2d] sleeping for %d seconds\n", num, sleepSeconds );
		fflush( stdout );
    }

	sleep( sleepSeconds );

	if (debug)
    {
		printf( "[%2d] awake\n", num );
		fflush( stdout );
    }
}

/*
 * cat_sleep()
 *
 * Simulates a cat sleeping for a random amount of time.
 * input: cat number
 */
void cat_sleep(int num)
{
	int sleepSeconds;

	sleepSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_CAT_SLEEP);

	if (debug)
    {
		printf( "[%2d] cat sleeping for %d seconds\n", num, sleepSeconds );
		fflush( stdout );
    }
	sleep( sleepSeconds );
	if (debug)
    {
		printf( "[%2d] cat awake\n", num );
		fflush( stdout );
    }
}

 
/*
 * sago_2_monkeyGrass_is_safe()
 *
 * Returns when it is safe for this lizard to cross from the sago
 * to the monkey grass.
 * input: lizard number
 */
void sago_2_monkeyGrass_is_safe(int num)
{
	if (debug)
    {
		printf( "[%2d] checking  sago -> monkey grass\n", num );
		fflush( stdout );
    }
	/*
	* Decrement semaphore
	*/
	if (sem_wait(&sem_liz) == 0)                                    // DM AJ
	{
		pthread_mutex_lock(&numLizardsCrossing);                // DM AJ
		numOfLizardsCrossing++;                                 // DM AJ
		pthread_mutex_unlock(&numLizardsCrossing);              // DM AJ
	}
	
	if (debug)
    {
		printf( "[%2d] thinks  sago -> monkey grass  is safe\n", num );
		fflush( stdout );
    }
}


/*
 * cross_sago_2_monkeyGrass()
 *
 * Delays for 2 seconds to simulate crossing from the sago to
 * the monkey grass. 
 * input: lizard number
 */
void cross_sago_2_monkeyGrass(int num)
{
	if (debug)
    {
		printf( "[%2d] crossing  sago -> monkey grass\n", num );
		fflush( stdout );
    }

	/*
	* One more crossing this way
	*/
	pthread_mutex_lock(&sago2grass);                // DM AJ
	numCrossingSago2MonkeyGrass++;
	pthread_mutex_unlock(&sago2grass);              // DM AJ
	/*
	* Checks for lizards crossing both ways
	*/
	if (numCrossingMonkeyGrass2Sago && UNIDIRECTIONAL)
    {
		printf( "\tCrash!  We have a pile-up on the concrete.\n" );
		printf( "\t%d crossing sago -> monkey grass\n", numCrossingSago2MonkeyGrass );
		printf( "\t%d crossing monkey grass -> sago\n", numCrossingMonkeyGrass2Sago );
		exit( -1 );
    }


	/*
	* It takes a while to cross, so simulate it
	*/
	sleep( CROSS_SECONDS );

	/*
	* That one seems to have made it
	*/
	pthread_mutex_lock(&sago2grass);                // DM AJ
	numCrossingSago2MonkeyGrass--;
	pthread_mutex_unlock(&sago2grass);              // DM AJ
}


/*
 * made_it_2_monkeyGrass()
 *
 * Tells others they can go now
 * input: lizard number
 */
void made_it_2_monkeyGrass(int num)
{
	/*
	* Whew, made it across
	*/
	if (debug)
    {
		printf( "[%2d] made the  sago -> monkey grass  crossing\n", num );
		fflush( stdout );
    }
	/*
	* Increment semaphore to allow a blocked thread to continue execution
	*/
	if (sem_post(&sem_liz) == 0)                                    // DM AJ
	{
		pthread_mutex_lock(&numLizardsCrossing);                // DM AJ
		numOfLizardsCrossing--;                                 // DM AJ
		pthread_mutex_unlock(&numLizardsCrossing);              // DM AJ
	}
}


/*
 * lizard_eat()
 *
 * Simulates a lizard eating for a random amount of time
 * input: lizard number
 */
void lizard_eat(int num)
{
	int eatSeconds;

	eatSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_LIZARD_EAT);

	if (debug)
    {
		printf( "[%2d] eating for %d seconds\n", num, eatSeconds );
		fflush( stdout );
    }

	/*
	* Simulate eating by blocking for a few seconds
	*/
	sleep( eatSeconds );

	if (debug)
    {
		printf( "[%2d] finished eating\n", num );
		fflush( stdout );
    }
}


/*
 * monkeyGrass_2_sago_is_safe()
 *
 * Returns when it is safe for this lizard to cross from the monkey
 * grass to the sago.
 * input: lizard number
 */
void monkeyGrass_2_sago_is_safe(int num)
{
	if (debug)
    {
		printf( "[%2d] checking  monkey grass -> sago\n", num );
		fflush( stdout );
    }
	/*
	* Decrement semaphore
	*/
	if (sem_wait(&sem_liz) == 0)                                    // DM AJ
	{
		pthread_mutex_lock(&numLizardsCrossing);                // DM AJ
		numOfLizardsCrossing++;                                 // DM AJ
		pthread_mutex_unlock(&numLizardsCrossing);              // DM AJ
	}

	if (debug)
    {
		printf( "[%2d] thinks  monkey grass -> sago  is safe\n", num );
		fflush( stdout );
    }
}



/*
 * cross_monkeyGrass_2_sago()
 *
 * Delays for 2 second to simulate crossing from the monkey
 * grass to the sago. 
 * input: lizard number
 */
void cross_monkeyGrass_2_sago(int num)
{
	if (debug)
    {
		printf( "[%2d] crossing  monkey grass -> sago\n", num );
		fflush( stdout );
    }

	/*
	* One more crossing this way
	*/
	pthread_mutex_lock(&grass2sago);                // DM AJ
	numCrossingMonkeyGrass2Sago++;
	pthread_mutex_unlock(&grass2sago);              // DM AJ
  
	/*
	* Checks for lizards crossing both ways
	*/
	if (numCrossingSago2MonkeyGrass && UNIDIRECTIONAL)
    {
		printf( "\tOh No!, the lizards have cats all over them.\n" );
		printf( "\t%d crossing sago -> monkey grass\n", numCrossingSago2MonkeyGrass );
		printf( "\t%d crossing monkey grass -> sago\n", numCrossingMonkeyGrass2Sago );
		exit( -1 );
    }

	/*
	* It takes a while to cross, so simulate it
	*/
	sleep( CROSS_SECONDS );

	/*
	* That one seems to have made it
	*/
	pthread_mutex_lock(&grass2sago);                // DM AJ
	numCrossingMonkeyGrass2Sago--;
	pthread_mutex_unlock(&grass2sago);              // DM AJ
}


/*
 * made_it_2_sago()
 *
 * Tells others they can go now
 * input: lizard number
 */
void made_it_2_sago(int num)
{
	/*
	* Whew, made it across
	*/
	if (debug)
    {
		printf( "[%2d] made the  monkey grass -> sago  crossing\n", num );
		fflush( stdout );
    }
	/*
	* Increment semaphore to allow a blocked thread to continue execution
	*/
	if (sem_post(&sem_liz) == 0)                                    // DM AJ
	{
		pthread_mutex_lock(&numLizardsCrossing);                // DM AJ
		numOfLizardsCrossing--;                                 // DM AJ
		pthread_mutex_unlock(&numLizardsCrossing);              // DM AJ
	}
}


