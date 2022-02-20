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

int main()
{

    char input;
    int isMain = 1;  // identifier of main process (children will be 0)
    int alarmNr = 0; // For incrementing the number/ID of the alarms

    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    char *currentTimeString = asctime(currentTime);
    printf("Welcome to the alarm clock! The current date and time is: %s\n", currentTimeString);

    while (input != 'e')
    {
        printf("--- Enter \"s\" (schedule), \"l\" (list), \"c\" (cancel) or \"x\" (exit): ---\n");
        char text[1];
        scanf(" %c", text);
        input = text[0];
        while (waitpid(-1, NULL, WNOHANG) > 0); // Removes zombie processes without blocking
        //char text[20] = {'1','9','/','0','2','/','2','0','2','2','-','1','0',':','5','3',':','0','5','\0'};
        
        if (input == 's')
        {
            printf("Schedule an alarm in the format \"dd/mm/yyyy-hh:mm:ss\":\n");
            char text[20];
            scanf(" %s", text);
            if (text[0] == 'e')
            {
                input = 'e';
                break;
            }
            char day[3] = {text[0], text[1], '\0'};
            char month[3] = {text[3], text[4], '\0'};
            char year[5] = {text[6], text[7], text[8], text[9], '\0'};
            char hour[3] = {text[11], text[12], '\0'};
            char minute[3] = {text[14], text[15], '\0'};
            char second[3] = {text[17], text[18], '\0'};
            struct tm newAlarmTime = {
                .tm_mday = atoi(day),
                .tm_mon = atoi(month) - 1,
                .tm_year = atoi(year) - 1900,
                .tm_hour = atoi(hour),
                .tm_min = atoi(minute),
                .tm_sec = atoi(second),
            };
            strftime(text, sizeof(text), "%d/%m/%Y %T", &newAlarmTime);
            time_t alarmTime = mktime(&newAlarmTime);
            printf("You scheduled an alarm at: %s, in %li seconds\n", text, alarmTime - time(NULL));
            alarmNr++;
            int fileDescriptor[2];
            if (pipe(fileDescriptor) == -1)
            {
                printf("Error ocurred while trying to open pipe\n");
                return 1;
            }
            isMain = fork();
            if (!isMain)
            {
                int pid = getpid();
                close(fileDescriptor[0]);
                write(fileDescriptor[1], &pid, sizeof(int));
                close(fileDescriptor[1]);
                sleep(alarmTime - time(NULL));
                printf("[Alarm %i: \"RINGGGG\"]\n", alarmNr);
                exit(0);
            }
            close(fileDescriptor[1]);
            int pid;
            read(fileDescriptor[0], &pid, sizeof(int));
            close(fileDescriptor[0]);
            struct Alarm newAlarm = {
                .alarmdID = alarmNr,
                .alarmTime = alarmTime,
                .pid = pid};
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
                    // alarms[i].isActive = 0;
                }
            }
        }
        else if (input == 'e')
        {
            break;
        }
        else if (input == 'l')
        {
            printf("Your alarms:\n");
            for (int i = 0; i < sizeof(alarms); i++)
            {
                if (alarms[i].alarmdID)
                {
                    char text[20];
                    strftime(text, sizeof(text), "%d/%m/%Y %T", localtime(&alarms[i].alarmTime));
                    printf(" - Alarm %i at %s   ", alarms[i].alarmdID, text);
                    int status;
                    if (waitpid(alarms[i].pid, &status, WNOHANG) == 0) // When child process is running
                    {
                        printf("(ACTIVE)\n");
                    }
                    else // When process has stopped
                    {
                        printf("(CANCELED)\n");
                    }
                }
            }
            //input = 'c';
        }
    }
    // Kill all child processes before ending the program
    signal(SIGQUIT, SIG_IGN);
    kill(0, SIGQUIT);

    return 0;
}