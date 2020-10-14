// RTOS Framework - Spring 2020
// J Losh

// Student Name:Rahul Kidecha
// TO DO: Add your name on this line.  Do not include your ID number in the file.

// Add 07_ prefix to all files in your project
// 07_rtos.c
// 07_tm4c123gh6pm_startup_ccs.c
// xx_other files (except uart0.x and wait.x)
// (xx is a unique number that will be issued in class)
// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// LEDs on these pins:
// Blue:   PF2 (on-board)
// Red:    PA2
// Orange: PA3
// Yellow: PA4
// Green:  PE0
// PBs on these pins
// PB0:    PC4
// PB1:    PC5
// PB2:    PC6
// PB3:    PC7
// PB4:    PD6
// PB5:    PD7
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "wait.h"


#define BLUE_LED (*((volatile uint32_t *)(0x42000000 + (0x400253FC - 0x40000000) * 32 + 2 * 4))) //PF2
#define GREEN_LED (*((volatile uint32_t *)(0x42000000 + (0x400243FC - 0x40000000) * 32 + 0 * 4))) //PE0
#define YELLOW_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC - 0x40000000) * 32 + 4 * 4))) //PA4
#define ORANGE_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC - 0x40000000) * 32 + 3 * 4))) //PA3
#define RED_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC - 0x40000000) * 32 + 2 * 4))) //PA2
#define POWER (*((volatile uint32_t *)(0x42000000 + (0x400253FC - 0x40000000) * 32 + 3 * 4))) //PF3

#define PB0 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 4 * 4)))
#define PB1 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 5 * 4)))
#define PB2 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 6 * 4)))
#define PB3 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 7 * 4)))
#define PB4 (*((volatile uint32_t *)(0x42000000 + (0x400073FC - 0x40000000) * 32 + 6 * 4)))
#define PB5 (*((volatile uint32_t *)(0x42000000 + (0x400073FC - 0x40000000) * 32 + 7 * 4)))


#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 1
#define YELLOW_LED_MASK 16
#define ORANGE_LED_MASK 8
#define RED_LED_MASK 4
#define POWER_LED_MASK 8

#define PB0_MASK 16     //PC4
#define PB1_MASK 32     //PC5
#define PB2_MASK 64     //PC6
#define PB3_MASK 128    //PC7
#define PB4_MASK 64     //PD6
#define PB5_MASK 128    //PD7

#define Buffer_Max  80  // Maximum size of input command buffer.
#define MAX_Args 6      // Maximum number of arguments for any available commands.

char input[Buffer_Max]; // Input buffer
char str[100];
char *commands[9] = {"ps","ipcs","pidof","kill","sched","pi","reboot","preemption","restore"};
char* mode[2]={"on","off"};

uint8_t argc,i,j,k,count;              // Argument count and general purpose variables.
uint8_t pos[MAX_Args];                // Position of arguments in input buffer.

// Assembly file
extern void set_PSP(uint32_t *PSP);
extern void* get_PSP();
extern void pushR4_11();
extern void popR4_11();


//-----------------------------------------------------------------------------
// String Function
//-----------------------------------------------------------------------------


void strcpy(char *destination, const char *source)   //copy string
{
    while(*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }

    *destination = '\0';
}

uint8_t strcmp(char *strg1, char *strg2) // compare sting
{
    while( ( *strg1 != '\0' && *strg2 != '\0' ) && *strg1 == *strg2 )
    {
        strg1++;
        strg2++;
    }

    if(*strg1 == *strg2)
    {
        return 0; // strings are identical
    }

    else
    {
        return *strg1 - *strg2; // reture difference
    }
}

void reverse(char str[], uint8_t length)  // Reverse String
{
    if (str == 0)                             /*!< Null was encountered */
    {
        return;
    }

    if (*str == 0)                            /*!< Empty String */
    {
        return;
    }

    char *start = str;
    char *end = start + length - 1;     /*!< Get size */
    char temp;

    while (end > start)
    {
        temp = *start;                       /*!< Copy string from end to front */
        *start = *end;
        *end = temp;

        ++start;                             /*!< Move pointers */
        --end;
    }
}


char* itoa(uint16_t num, char* str, uint16_t base)  // decimal to ASCII
{
    uint16_t i = 0;

        if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    while (num != 0)
    {
        uint8_t rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}

int atoi(const char *str)  //ASCII to Decimal
{
    if (*str == '\0')
       return 0;

    uint16_t result = 0;        // Initialize result

    while (*str >= '0' && *str <= '9')
    {
        result = (10 * result) + (*str - '0');
        ++str;
    }

    return result;
}



//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// semaphore
#define MAX_SEMAPHORES 5
#define MAX_QUEUE_SIZE 5
struct semaphore
{
    uint16_t count;
    uint16_t queueSize;
    uint32_t processQueue[MAX_QUEUE_SIZE]; // store task index here
    char semaname[16];                      // semaname
} semaphores[MAX_SEMAPHORES];
uint8_t semaphoreCount = 0;

struct semaphore *keyPressed, *keyReleased, *flashReq, *resource;

// task
#define STATE_INVALID    0 // no task
#define STATE_UNRUN      1 // task has never been run
#define STATE_READY      2 // has run, can resume at any time
#define STATE_DELAYED    3 // has run, but now awaiting timer
#define STATE_BLOCKED    4 // has run, but now blocked by semaphore

#define MAX_TASKS 10       // maximum number of valid tasks
uint8_t taskCurrent = 0;   // index of last dispatched task
uint8_t taskCount = 0;     // total number of valid tasks
uint32_t stack[MAX_TASKS][512]; //2048 byte for each task


#define svc_yield 25
#define svc_sleep 26
#define svc_wait  27
#define svc_post 28
#define svc_kill 5


// REQUIRED: add store and management for the memory used by the thread stacks
//           thread stacks must start on 1 kiB boundaries so mpu can work correctly

struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread
    void *spInit;                  // location of original stack pointer
    void *sp;                      // location of stack pointer for thread
    int8_t priority;               // 0=highest to 15=lowest
    int8_t currentPriority;           // used for priority inheritance
    uint32_t ticks;                // ticks until sleep complete
    char name[16];                 // name of task used in ps command
    void *semaphore;               // pointer to the semaphore that is blocking the thread
} tcb[MAX_TASKS];

// time calculations
struct timeCalc
{
    uint32_t runTime;                                  // Used for storing time taken by task to run
    uint32_t filterTime;                               // Used for storing the moving averaged value of run time
    uint32_t taskPercentage;                           // User for storing value of CPU usage for each task

}processTime[MAX_TASKS];

// Time Calculation Variables
uint32_t startTime        = 0;                         // User for storing start time of thread
uint32_t stopTime         = 0;                         // User for storing stop time of thread
uint32_t totalTime        = 0;                         // User for storing total time taken
uint16_t measureTimeSlice = 0;                         // User for storing value of measurement time slice in SystickISR

//Schedule Priority
uint8_t schedPriority = 1;                      // schedule Priority flag default in Priority mode

uint8_t preempt = 1;                            // Default preemption is on

uint8_t pi = 0;                                 //Priority Inheritance is on default

//-----------------------------------------------------------------------------
// RTOS Kernel Functions
//-----------------------------------------------------------------------------

// Reboot function
void Reset()
{
    putsUart0("\r\n Restarting system... \r\n");
    waitMicrosecond(1000000);
    NVIC_APINT_R = 0x04 | (0x05FA << 16);
 }


// REQUIRED: initialize systick for 1ms system timer
void initRtos()
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }

}

// Calculate the CPU time for task
void timer1()
{
    uint8_t tsk;
    uint8_t firstUpdate = 1;

    measureTimeSlice++;
    if(measureTimeSlice==100)
    {
    for(tsk=0; tsk<MAX_TASKS; tsk++)
    {
        if(firstUpdate)
        {
            processTime[tsk].filterTime = processTime[tsk].runTime;
            firstUpdate = 0;
        }
        else
         // processTime[tsk].filterTime = processTime[tsk].filterTime * 0.8 + processTime[tsk].runTime * (1 - 0.8);     // Alpha = 0.9
         processTime[tsk].filterTime = (processTime[tsk].filterTime * 7 + processTime[tsk].runTime)>>3;

    }


    totalTime = 0;
    // Calculate Total Time
    for(tsk=0; tsk<MAX_TASKS; tsk++)
    {
        totalTime = totalTime + processTime[tsk].filterTime;
    }
    // Calculate CPU %age
    for(tsk=0; tsk<MAX_TASKS; tsk++)
    {
        processTime[tsk].taskPercentage = (processTime[tsk].filterTime * 10000) / totalTime;       // Multiply by 10000 to preserve decimal upto two place

        processTime[tsk].runTime = 0;                                                              // clear runTime variable
    }
    measureTimeSlice = 0;
    }
}

// REQUIRED: Implement prioritization to 16 levels
int rtosScheduler()
{
    bool ok;
    static uint8_t task = 0xFF;
    static uint8_t cnt = 0;
    uint8_t priority = 0;
    ok = false;
    while (!ok)
    {
        task++;
        if (task >= MAX_TASKS)

            task = 0;

        if(schedPriority==0)  // Round Robin
            {
            ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
            }
        if (schedPriority==1) //priority scheduler
            {
            for(priority=0;priority<16;priority++)

            for (i=1; i <= MAX_TASKS; i++)
                {
                uint8_t val;
                val = (i+cnt-1)%MAX_TASKS;
                if(tcb[val].currentPriority == priority)
                    {
                        if (tcb[val].state == STATE_READY || tcb[val].state == STATE_UNRUN)
                        {
                            cnt=val+1;
                            return val;
                        } // state condition check

                    } // Priority match

                }//priority and task loop
            }
    }
    return task;
}

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0;
    bool found = false;
    // REQUIRED: store the thread name
    // add task if room in task list
    // allocate stack space for a thread and assign to sp below
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            tcb[i].state = STATE_UNRUN;
            tcb[i].pid = fn;
            tcb[i].spInit = &stack[i][511];
            tcb[i].sp = &stack[i][511];  // point to top of the stack
            tcb[i].priority = priority;
            tcb[i].currentPriority = priority;
            strcpy(tcb[i].name, name);
            // increment task count
            taskCount++;
            ok = true;
        }
    }
    // REQUIRED: allow tasks switches again
    return ok;
}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
for (j=0;j<MAX_TASKS;j++)
 if (strcmp(fn,tcb[j].pid)==0)
 {
 tcb[j].state = STATE_UNRUN;
 tcb[j].sp = tcb[j].spInit;
 putsUart0("restore is Enabled \r\n");
 break;
 }

}

// REQUIRED: modify this function to destroy a thread
// REQUIRED: remove any pending semaphore waiting
void destroyThread(_fn fn)
{
    __asm(" SVC #5");
}

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
    uint8_t k;
    for(k = 0; k < taskCount; k++)
    {
        if(tcb[k].pid == fn)
            break;
    }
    tcb[k].priority = priority;
    tcb[k].currentPriority = priority;


}

struct semaphore* createSemaphore(uint8_t count,char name[])
{
    struct semaphore *pSemaphore = 0;
    if (semaphoreCount < MAX_SEMAPHORES)
    {
        pSemaphore = &semaphores[semaphoreCount++];
        pSemaphore->count = count;
        strcpy(pSemaphore->semaname, name);
    }
    return pSemaphore;
}

// REQUIRED: modify this function to start the operating system, using all created tasks
void startRtos()
{

      NVIC_ST_CTRL_R = 0;      // Disable the sys timer before initializing with 1ms
      NVIC_ST_RELOAD_R = 0x9C3F; // (40mhz / 1000hz) - 1  = 39,999 = 0x9c3f
      NVIC_ST_CURRENT_R = 0;// reset current
            NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN
              | NVIC_ST_CTRL_ENABLE; // Enable timer
      //NVIC_ST_CTRL_R |= 0x00000007;


    _fn fn;
    taskCurrent = rtosScheduler();            // Call scheduler
    TIMER1_CTL_R |= TIMER_CTL_TAEN; // calculate for 1st task
     set_PSP(tcb[taskCurrent].sp);                //set sp
    fn = (_fn)tcb[taskCurrent].pid;            //set pc
    (*fn)();

}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
// push registers, call scheduler, pop registers, return to new function
void yield()
{
    __asm(" SVC #25");
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
// push registers, set state to delayed, store timeout, call scheduler, pop registers,
// return to new function (separate unrun or ready processing)
void sleep(uint32_t tick)
{
    __asm(" SVC #26");
}

// REQUIRED: modify this function to wait a semaphore with priority inheritance
// return if avail (separate unrun or ready processing), else yield to scheduler using pendsv
void wait(struct semaphore *pSemaphore)
{
    __asm(" SVC #27");
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(struct semaphore *pSemaphore)
{
    __asm(" SVC #28");
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr()
{
uint8_t cnt;
if (preempt==1)  // if Preemption is on
{
    NVIC_INT_CTRL_R |= 0x10000000;
}

for (cnt=0;cnt<taskCount;cnt++){
if (tcb[cnt].state == STATE_DELAYED){

    tcb[cnt].ticks--;

            if(tcb[cnt].ticks == 0) {
                tcb[cnt].state = STATE_READY;
            }
}
}
timer1();  // calculate the CPU time %
}
// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
void pendSvIsr()
{
    pushR4_11();
    tcb[taskCurrent].sp = get_PSP();
//    __asm(" MOV R4, #8"    );
//    __asm(" MOV R5, #8"    );
//    __asm(" MOV R6, #8"    );
//    __asm(" MOV R7, #8"    );
//    __asm(" MOV R8, #8"    );
//    __asm(" MOV R9, #8"    );
//    __asm(" MOV R10, #8"    );
//    __asm(" MOV R11, #8"    );

    // stop the timer
    stopTime = TIMER1_TAV_R;                                              // Save Final Value of Timer register
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                                      // Stop Timer
    TIMER1_TAV_R = 0;                                                     // Set Timer Zero

    // Calculate time difference

        processTime[taskCurrent].runTime = stopTime - startTime;          // Store Time Difference
        stopTime = 0;
        startTime = 0;

    taskCurrent = rtosScheduler();

    // Time Measurements Start
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                                       // Start the timer
    startTime = TIMER1_TAV_R;                                             // Save initial Value of Timer register

   if(tcb[taskCurrent].state == STATE_READY)
    {
       set_PSP(tcb[taskCurrent].sp);
        popR4_11();
    }
    else
    {
        set_PSP(tcb[taskCurrent].sp);

        uint32_t *p=get_PSP();
         p--;
         *p = 0x01000000;  //set XPSR to thumb instructions
         p--;
         *p = (uint32_t)tcb[taskCurrent].pid;
         p=p-6;
         set_PSP(p);

        tcb[taskCurrent].state = STATE_READY ;
    }

    //GREEN_LED ^= 1;
      //waitMicrosecond(500000);
     // GREEN_LED = 0;
}

// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr()
{
    uint32_t *PC;
    uint8_t i;
    PC = get_PSP();
    PC = PC+6;
    uint16_t *SVC_number;
    SVC_number=*PC-2;
    uint16_t num;
    num=*SVC_number & 0x00FF;

    switch(num)

    {
    case svc_yield:
        NVIC_INT_CTRL_R |= 0x10000000;  // call pendsvcISR
        break;

    case svc_sleep:
        tcb[taskCurrent].state = STATE_DELAYED;
        uint32_t *p=get_PSP();
        tcb[taskCurrent].ticks = *p;
        NVIC_INT_CTRL_R |= 0x10000000;  // Call pendSvcISR
        break;

    case svc_wait:
        p = get_PSP();
        struct semaphore *semapointer = (struct semaphore*)*p;
         if (semapointer->count > 0){
             semapointer->count--;
            // tcb[taskCurrent].semaphore = NULL;

         }else{
               semapointer->processQueue[semapointer->queueSize]=taskCurrent;  // Add current task to process Queue
               semapointer->queueSize++;
               tcb[taskCurrent].state = STATE_BLOCKED;
               tcb[taskCurrent].semaphore = semapointer;  // Record block semaphore

               // Priority Inheritance

               if(pi==1)
               {
                   for(i=0; i<MAX_TASKS; i++)                                                // Find previous user of this semaphore
                   {
                        if(tcb[i].semaphore == tcb[taskCurrent].semaphore)                    // check who else uses the same semaphore, while current task is blocked
                      {
                           if(tcb[i].currentPriority > tcb[taskCurrent].currentPriority)     // if found then check if priority value is greater than task current
                           {
                               tcb[i].currentPriority = tcb[taskCurrent].currentPriority;    // if yes, then give the priority value of task current(lower) to other user
                           }
                           break;
                       }
                   }
               }

               NVIC_INT_CTRL_R |= 0x10000000;  // Call pendSvcISR

         }
         break;

    case svc_post:
        p = get_PSP();
        semapointer = (struct semaphore*)*p;
        semapointer->count++;

        tcb[taskCurrent].currentPriority = tcb[taskCurrent].priority; //Priority Restore

        if(semapointer->queueSize > 0)
        {
            tcb[semapointer->processQueue[0]].state = STATE_READY;
          //  semapointer->currentUser = semapointer->processQueue[0];

            for (i=0;i<(semapointer->queueSize);i++)
            {
                semapointer->processQueue[i] = semapointer->processQueue[i+1];
            }
          //  semapointer->processQueue[semapointer->queueSize] = 0;
            semapointer->queueSize--;
            semapointer->count--;
        }
        break;
    case svc_kill:

        p=get_PSP();
        //semapointer = (struct semaphore*)*p;
        uint32_t task_pid = *p;

        for(i=0;i<MAX_TASKS;i++)
        {
            if(tcb[i].pid==(_fn)task_pid)
            {
                tcb[i].state = STATE_INVALID;
                semapointer = tcb[i].semaphore;

                if(semapointer!=0)
                {
                for (j=0;j<(semapointer->queueSize);j++)
                    {
                    semapointer->processQueue[j] = semapointer->processQueue[j+1];
                    }
                semapointer->queueSize--;
                }
            }
        }
        break;
    }

}

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           6 pushbuttons, and uart
void initHw()
{

    // Configure HW to work with 16 MHz XTAL, PLL enabled, sysdivider of 5, creating system clock of 40 MHz
       SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

       // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
       SYSCTL_GPIOHBCTL_R = 0;

       // SSI2 Configuration
//       SYSCTL_RCGCSSI_R = SYSCTL_RCGCSSI_R2; // turn-on SSI2 clocking

       // Enable GPIO port F peripherals
       SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOE | SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD;

       GPIO_PORTD_LOCK_R=  0x4C4F434B;
       GPIO_PORTD_CR_R = PB5_MASK;

       // Configure LED pin
       GPIO_PORTF_DIR_R = BLUE_LED_MASK |POWER_LED_MASK ;  // make bit an output
       GPIO_PORTF_DR2R_R = BLUE_LED_MASK |POWER_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
       GPIO_PORTF_DEN_R = BLUE_LED_MASK |POWER_LED_MASK;  // enable LED

       GPIO_PORTE_DIR_R = GREEN_LED_MASK ;  // make bit an output
       GPIO_PORTE_DR2R_R = GREEN_LED_MASK ; // set drive strength to 2mA (not needed since default configuration -- for clarity)
       GPIO_PORTE_DEN_R = GREEN_LED_MASK;  // enable LED

       GPIO_PORTA_DIR_R = YELLOW_LED_MASK |RED_LED_MASK |ORANGE_LED_MASK  ;  // make bit an output
       GPIO_PORTA_DR2R_R = YELLOW_LED_MASK |RED_LED_MASK |ORANGE_LED_MASK ; // set drive strength to 2mA (not needed since default configuration -- for clarity)
       GPIO_PORTA_DEN_R = YELLOW_LED_MASK |RED_LED_MASK |ORANGE_LED_MASK ;  // enable LED

       GPIO_PORTC_DEN_R = PB0_MASK | PB1_MASK | PB2_MASK | PB3_MASK;  // enable LEDs and pushbuttons
       GPIO_PORTC_PUR_R = PB0_MASK | PB1_MASK | PB2_MASK | PB3_MASK; // enable internal pull-up for push button

       GPIO_PORTD_DEN_R = PB4_MASK | PB5_MASK;  // enable LEDs and pushbuttons
       GPIO_PORTD_PUR_R = PB4_MASK | PB5_MASK; // enable internal pull-up for push button

       // Configure UART0 pins

          SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;         // turn-on UART0, leave other uarts in same status
          GPIO_PORTA_DEN_R |= 3;                           // default, added for clarity
          GPIO_PORTA_AFSEL_R |= 3;                         // default, added for clarity
          GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;

       // Configure UART0 to 115200 baud, 8N1 format (must be 3 clocks from clock enable and config writes)

          UART0_CTL_R = 0;                                 // turn-off UART0 to allow safe programming
          UART0_CC_R = UART_CC_CS_SYSCLK;                  // use system clock (40 MHz)
          UART0_IBRD_R = 21;                               // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
          UART0_FBRD_R = 45;                               // round(fract(r)*64)=45
          UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN; // configure for 8N1 w/ 16-level FIFO
          UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN; // enable TX, RX, and module

       //Configure Timer 1
          SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;        // clock to timer
          TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
          TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
          TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD | TIMER_TAMR_TACDIR;        // configure for periodic mode (count up)

}

//-----------------------------------------------------------------------------
// UART subroutines
//-----------------------------------------------------------------------------

// Blocking function that writes a serial character when the UART buffer is not full
void putcUart0(char c)
{
    while ((UART0_FR_R & UART_FR_TXFF)!=0) //Blocking but can yield while it's blocking
    {
        yield();
    }
    UART0_DR_R = c;
}

// Blocking function that writes a string when the UART buffer is not full
void putsUart0(char* str)
{
    uint8_t i = 0;
    while (str[i] != '\0')

        putcUart0(str[i++]);
}

// Blocking function that returns with serial data once the buffer is not empty
char getcUart0()
{
    while ((UART0_FR_R & UART_FR_RXFE)!=0)  //Blocking but can yield while it's blocking
    {
        yield();
    }
    return UART0_DR_R & 0xFF;
}


// REQUIRED: add code to return a value from 0-63 indicating which of 6 PBs are pressed
uint8_t readPbs()
{
    uint8_t buttonOUT = 0;

    if(PB0 == 0)
        buttonOUT |= 1;

    if(PB1 == 0)
        buttonOUT |= 2;

    if(PB2 == 0)
        buttonOUT |= 4;

    if(PB3== 0)
        buttonOUT |= 8;

    if(PB4 == 0)
        buttonOUT |= 16;

    if(PB5 == 0)
        buttonOUT |= 32;

    return buttonOUT;
}

void getsUart0()
{

     uint8_t c;
     uint8_t count = 0;
A1:  c = getcUart0();
     putcUart0(c);
     if (c == 127) { //check backspace
         if (count == 0)// Checking count
             goto A1;

         else if (count > 0)
         {
              count = count-1;
              goto A1;
         }
     }
      else
          {
         if(c != 0x0D) //  Enter terminates entry mode.
         {
             if(c < 0x20)  // Check for printability of character.
                 goto A1;
             else
            {
             input[count] = c; // All entries to the input buffer are converted to lowercase
             count++;
             if (count < Buffer_Max)     // Checking if buffer is full.
             {
                 goto A1;

             }
             else {
                 putsUart0("Buffer is full\r\n\r\n");
                 return;
             }
            }
         }
         else
         {
             input[count] = '\0';
             return;
         }
     }
}


// Parse string function
void parsestring()
{

j = 0;
k = 0;
bool flag = true;

for(i=0;i<input[i]!='\0';i++)     // Loop to parse the entries after the first entry.
{
if(input[i]==32 || input[i] ==',') // check space or coma
{
    input[i] = '\0';
    flag = true;
    continue;
}
if(96<(input[i])<123|| 47<(input[i])<58 || input[i] == '.'||input[i] == '-') // digit/small letters/. and -
{
    if (flag)
    {
        argc++;
        pos[j++] = i;
        flag = false;
    }
}
}
}

// pid of thread
void pidthread()
{
    char str[100];
    uint8_t r;
    bool c = 0;
    for(r = 0 ; r < MAX_TASKS ; r++)
    {
        if(strcmp(&input[pos[1]],tcb[r].name)==0)
        {
        c = 1;
        break;
        }
    }
    if(c)
    {
    putsUart0("pid is");
    putsUart0(itoa(tcb[r].pid, str, 10));
    putsUart0("\r\n");
    }
    else
        putsUart0("Thread doesn't exist\r\n");

}
// IPCS code

void ipcs()
{
putsUart0("    Name     | Count |    Waiting-Task   \r\n");
putsUart0("-------------|-------|-------------------\r\n");
uint8_t i;
for( i = 0 ; i < MAX_SEMAPHORES-1 ; i++)
{
    putsUart0(semaphores[i].semaname);
    putsUart0(" \t");
    putsUart0(itoa(semaphores[i].count, str, 10));
    putsUart0(" \t");

    if(semaphores[i].processQueue[0] == 0)
        putsUart0(" None \t");
    else
    {
        putsUart0(tcb[semaphores[i].processQueue[0]].name);
        putsUart0(" \t");
    }
    putsUart0("\r\n");
}
}

// PS

void ps()
{
    uint32_t cpuP = 0;
    uint32_t cpuNUM = 0;

    putsUart0("    Name     |     PID     |    Priority    |    State     |    %CPU      \r\n");
    putsUart0("-------------|-------------|----------------|--------------|--------------\r\n");
    uint8_t i = 0;
    for(i = 0 ; (i < taskCount); i++)
    {


        if(tcb[i].pid == 0)
            continue;
        putsUart0(tcb[i].name);
        putsUart0("     \t");
        putsUart0(itoa(tcb[i].pid, str, 10));
        putsUart0("    \t   ");
        putsUart0(itoa(tcb[i].priority, str, 10));
        putsUart0("    \t");

        switch(tcb[i].state)
        {
        case 0:
            putsUart0("INVALID");
            break;

        case 1:
            putsUart0("UNRUN");
            break;

        case 2:
            putsUart0("READY");
            break;

        case 3:
            putsUart0("DELAYED");
            break;

        case 4:
            putsUart0("BLOCKED");
            break;

        default:
            break;
        }
        putsUart0("    \t");

        cpuP = processTime[i].taskPercentage;
        cpuNUM = cpuP / 100;    // Get Characteristic Part

        if(cpuNUM < 10)
            putsUart0("0");  // print 1st digit zero when less than 10


        putsUart0(itoa(cpuNUM, str, 10));
        cpuNUM = 0;
        putcUart0('.');         // Decimal point representation
        cpuNUM = cpuP % 100;    // Get Mantissa Part
        putsUart0(itoa(cpuNUM, str, 10));      // Print Mantissa part

        if(cpuNUM < 10)
            putsUart0("0");
        cpuNUM = 0;



        putsUart0("    \t");
        putsUart0(" \r\n");
    }

}
// char *commands[9] = {"ps","ipcs","pidof","kill","sched","pi","reboot","preemption","restore"};

char IsCommand()
{

    if(strcmp(&input[pos[0]],commands[0])==0){ // PS
        putsUart0("Input string is ps \r\n");
        ps();
      }else if (strcmp(&input[pos[0]],commands[1])==0){ //IPSC
         putsUart0("Input string is ipcs \r\n");
          ipcs();
     }else  if (strcmp(&input[pos[0]],commands[2])==0){ // pid of
         putsUart0("Input string is pid of \r\n");
          pidthread();
     }else  if (strcmp(&input[pos[0]],commands[3])==0){ // kill
         putsUart0("kill is Enabled \r\n");
         uint32_t rec_pid =atoi(&input[pos[1]]);
         destroyThread((_fn)rec_pid);
     }else if (strcmp(&input[pos[0]],commands[4])==0){ // scheduler

         if (strcmp(&input[pos[1]],mode[1])==0){ //rr (off)
             schedPriority = 0;
             putsUart0("Round Robin Enabled \r\n");

         }else if (strcmp(&input[pos[1]],mode[0])==0){ //priority (on)
             schedPriority = 1;
             putsUart0("Priority Enabled \r\n");
         }
     }else if (strcmp(&input[pos[0]],commands[7])==0){

         if (strcmp(&input[pos[1]],mode[1])==0){ //preemption (off)
             preempt = 0;
             putsUart0("Preemption disabled \r\n");

         }else if (strcmp(&input[pos[1]],mode[0])==0){ //preemption (on)
             preempt = 1;
             putsUart0("Preemption Enabled \r\n");
         }
     }else if(strcmp(&input[pos[0]],commands[5])==0){  //Priority Inheritance
         if (strcmp(&input[pos[1]],mode[1])==0){ //pi (off)
             pi = 0;
             putsUart0("Pi disabled \r\n");

         }else if (strcmp(&input[pos[1]],mode[0])==0){ //pi (on)
             pi = 1;
             putsUart0("Pi Enabled \r\n");
         }
     }else if(strcmp(&input[pos[0]],commands[8])==0){ // restore
         for (i=0;i<MAX_TASKS;i++)
           { if(tcb[i].state==STATE_INVALID)
               { if(strcmp(&input[pos[1]],tcb[i].name)==0)
                 break;
               }
          }
          restartThread(tcb[i].pid);
     }else if (strcmp(&input[pos[0]],commands[6])==0){  //reset
         Reset();
     }else{
         putsUart0("Invalid Command \r\n");
     }

return 0;
}



//-----------------------------------------------------------------------------
// YOUR UNIQUE CODE
// REQUIRED: add any custom code in this space
//-----------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Task functions
// ------------------------------------------------------------------------------

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle()
{
//    __asm(" MOV R4, #4"    );
//    __asm(" MOV R5, #5"    );
//    __asm(" MOV R6, #6"    );
//    __asm(" MOV R7, #7"    );
//    __asm(" MOV R8, #8"    );
//    __asm(" MOV R9, #9"    );
//    __asm(" MOV R10, #10"    );
//    __asm(" MOV R11, #11"    );

    while(true)
    {
        ORANGE_LED = 1;
        waitMicrosecond(1000);
        ORANGE_LED = 0;
        yield();

    }
}

//void idle_2()
//{
//
//    while(true)
//    {
//        YELLOW_LED = 1;
//        waitMicrosecond(3000);
//        YELLOW_LED = 0;
//
//        yield();
//    }
//}

void flash4Hz()
{
    while(true)
    {
        GREEN_LED ^= 1;
        sleep(125);
    }
}

void oneshot()
{
    while(true)
    {
        wait(flashReq);
        YELLOW_LED = 1;
        sleep(1000);
        YELLOW_LED = 0;
    }
}

void partOfLengthyFn()
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();

}

void lengthyFn()
{
    uint16_t i;
    while(true)
    {
        wait(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        RED_LED ^= 1;
        post(resource);
    }

}

void readKeys()
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)  // toggle yellow and red led PB0
        {
            YELLOW_LED ^= 1;
            RED_LED = 1;
        }
        if ((buttons & 2) != 0) // post flashReq so oneshot get semaphore PB1
        {
            post(flashReq);
            RED_LED = 0;
        }
        if ((buttons & 4) != 0) // recreat thread PB2
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0) // destroy thread PB3
        {
            destroyThread(flash4Hz);
        }
        if ((buttons & 16) != 0) // set lengthyfn to high Priority PB4
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }

}

void debounce()
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }

}

void uncooperative()
{
    while(true)
    {
        while (readPbs() == 32) // PB5
        {
        }
        yield();
    }
}

void important()
{
    while(true)
    {
        wait(resource);
        BLUE_LED = 1;
        sleep(1000);
        BLUE_LED = 0;
        post(resource);
    }
}

// REQUIRED: add processing for the shell commands through the UART here
//           your solution should not use C library calls for strings, as the stack will be too large
void shell()
{
    putsUart0("\033[2J\033[H"); // clear screen

    putsUart0("\r\n");
    putsUart0(">> 6314 - Advanced Microcontroller Project");
    putsUart0("\r\n");


    while (true)
    {
        argc = 0;
        getsUart0();

        parsestring(); // Parse input string

//                for (i=0;i<j;i++) // to display the Arguments
//               {
//                  putsUart0("Argument is :");
//                  putsUart0(&input[pos[i]]);
//                  putsUart0("\r\n");
//                  putsUart0("\r\n");
//                     }

         IsCommand();
         putsUart0("\r\n");
         yield();
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    bool ok;

    // Initialize hardware
    initHw();
    initRtos();

    // Power-up flash
    GREEN_LED = 1;
    waitMicrosecond(250000);
    GREEN_LED = 0;
    waitMicrosecond(250000);

    // Initialize semaphores
    keyPressed = createSemaphore(1,"KeyPressed");
    keyReleased = createSemaphore(0,"KeyReleased");
    flashReq = createSemaphore(5,"flashReq");
    resource = createSemaphore(1,"resource");

    // Add required idle process at lowest priorityse
    ok =  createThread(idle, "Idle", 15, 1024);  //0
  // ok &=  createThread(idle_2, "Idle_2", 15, 1024);
    // Add other processes
    ok &= createThread(lengthyFn, "LengthyFn", 12, 1024); //1
    ok &= createThread(flash4Hz, "Flash4Hz", 8, 1024); //2
    ok &= createThread(oneshot, "OneShot", 4, 1024); //3
    ok &= createThread(readKeys, "ReadKeys", 12, 1024); //4
    ok &= createThread(debounce, "Debounce", 12, 1024); //5
    ok &= createThread(important, "Important", 0, 1024); //6
    ok &= createThread(uncooperative, "Uncoop", 12, 1024); //7
    ok &= createThread(shell, "Shell", 12, 1024); //8

    // Start up RTOS
    if (ok)
        startRtos(); // never returns
    else
        RED_LED = 1;

    return 0;
}
