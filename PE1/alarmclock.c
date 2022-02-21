#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

struct Alarm
{
    int alarmdID;
    int pid;
    time_t alarmTime;
};

struct Alarm alarms[10];

struct Alarm setAlarm(int *alarmNr)
{
    printf("Schedule an alarm in the format \"dd/mm/yyyy-hh:mm:ss\":\n");
    char text[20];
    scanf(" %s", text);

    struct tm newAlarmTime;
    strptime(text, "%d/%m/%Y-%T", &newAlarmTime);
    time_t alarmTime = mktime(&newAlarmTime);

    printf("You scheduled an alarm at: %s, in %li seconds\n", text, alarmTime - time(NULL));
    (*alarmNr)++; // Increments alarmNr for each alarm

    int fileDescriptor[2]; // Pipe for communicating pid of child to parent
    if (pipe(fileDescriptor) == -1)
    {
        printf("Error ocurred while trying to open pipe\n");
        // return 1;
    }
    int isMain = fork();
    if (!isMain)
    {
        int pid = getpid();
        close(fileDescriptor[0]);
        write(fileDescriptor[1], &pid, sizeof(int));
        close(fileDescriptor[1]);
        sleep(alarmTime - time(NULL));
        printf("[Alarm %i: \"RINGGGG\"]\n", *alarmNr);
        system("afplay alarm.mp3 -v 1"); // Play sound on Mac, comment out if not working
        exit(0);
    }
    close(fileDescriptor[1]);
    int pid;
    read(fileDescriptor[0], &pid, sizeof(int));
    close(fileDescriptor[0]);

    struct Alarm newAlarm = {
        .alarmdID = *alarmNr,
        .alarmTime = alarmTime,
        .pid = pid};
    
    return newAlarm;
}

int main()
{

    char input;
    // int isMain = 1;  // identifier of main process (children will be 0)
    int alarmNr = 0; // For incrementing the number/ID of the alarms

    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    char *currentTimeString = asctime(currentTime);
    printf("Welcome to the alarm clock! The current date and time is: %s\n", currentTimeString);

    while (input != 'x')
    {
        printf("--- Enter \"s\" (schedule), \"l\" (list), \"c\" (cancel) or \"x\" (exit): ---\n");
        char text[1];
        scanf(" %c", text);
        input = text[0];
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ; // Removes zombie processes without blocking

        if (input == 's')
        {
            struct Alarm newAlarm = setAlarm(&alarmNr);
            alarms[alarmNr - 1] = newAlarm;
        }
        else if (input == 'c')
        {
            printf("Enter the number of the alarm you want to cancel:\n");
            int cancelNr;
            scanf("%d", &cancelNr);
            for (int i = 0; i < sizeof(alarms); i++)
            {
                if (cancelNr == alarms[i].alarmdID)
                {
                    kill(alarms[i].pid, SIGTERM);
                }
            }
        }
        else if (input == 'l')
        {
            printf("Your alarms:\n");
            for (int i = 0; i < sizeof(alarms); i++)
            {
                // Check if alarm exists
                if (alarms[i].alarmdID)
                {
                    char text[20];
                    strftime(text, sizeof(text), "%d/%m/%Y %T", localtime(&alarms[i].alarmTime));
                    printf(" - Alarm %i at %s   ", alarms[i].alarmdID, text);
                    // Check if child process is running
                    if (waitpid(alarms[i].pid, NULL, WNOHANG) == 0)
                    {
                        printf("(ACTIVE)\n");
                    }
                    else
                    {
                        printf("(INACTIVE)\n");
                    }
                }
            }
        }
    }
    // Kill all running child processes before ending the program
    for (int i = 0; i < sizeof(alarms); i++)
    {
        // Check if process exists in alarms[]
        if (alarms[i].alarmdID)
        {
            // Check if process is running
            if (waitpid(alarms[i].pid, NULL, WNOHANG) == 0)
            {
                kill(alarms[i].pid, SIGTERM);
            }
        }
    }
    // Remove potential zombies
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    return 0;
}