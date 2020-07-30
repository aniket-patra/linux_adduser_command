/*
Aniket Patra 12/May/2020 07:54AM https://aniket-patra.github.io/

Hello, I am writing down the details of this program here:
I used: https://www.cyberciti.biz/faq/understanding-etcshadow-file/  (For Shadow file structure) and https://www.cyberciti.biz/faq/understanding-etcpasswd-file-format/  (For passwd file structure)
I have provided comments for everything and everywhere possible.
I have assumed few things like encryption method in shadow file then default path in both shadow and passwd file

This program mimics the original linux adduser command as closely as possible and it also prints all the messages as displayed by linux nearly the same.
For further information please check the in-program comments
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat st = {0};

void extractInfo(char *envp[], char array[50], char string[6], int size) //function to extract different path values from environment variables
{
    // Use current time as seed for random generator
    srand(time(0));
    int i, j, k, flag;
    char c, x[6];
    for (i = 0; envp[i] != NULL; i++) //extracting environment info. from envp into char var
    {
        flag = 0;
        for (j = 0; j < size; j++)
            x[j] = envp[i][j];
        x[j] = 0;
        array[0] = 0;
        if (!strcmp(x, string))
        {
            flag = 1;
            while (1)
            {
                if (envp[i][j] == '=') //copying everything that comes after '=' (path values)
                {
                    k = 0;
                    while ((c = envp[i][++j]) != 0)
                    {
                        array[k++] = c;
                    }
                    array[k] = c;
                    break;
                }
            }
        }
        if (flag == 1)
            break;
    }
}

int main(int argc, char *argv[], char *envp[])
{
    //for lastchanged field in shadow file
    time_t seconds;
    seconds = time(NULL);
    char encrypt, c, x[6], SHELL[50], PWD[50], PFILE[50], PATH[50], P_PATH[50], S_PATH[50], password[15], password2[15]; //P_PATH for passwd, S_PATH for shadow, and PATH for home address
    char fullname[15], roomnumber[5], workphone[10], homephone[10], other[20];                                           //passwd file arguments
    FILE *passwd, *shadow;
    int UID, GID, flag = 0, i, min, max, warn, inactive, expire; //shadow file and passwd file arguments (check topmost comment)

    if (argc < 2) //if sufficient no. of arguments not provided
    {
        printf("%s: %s\n", argv[0], "Expecting adduser command.");
        return 0;
    }
    else if (argc == 2 && !strcmp(argv[1], "adduser")) //if it is adduser only (without any name(s))
    {
        printf("%s\n", "adduser: Only one or two names allowed.");
        return 0;
    }
    else if (argc > 4 && !strcmp(argv[1], "adduser") && strcmp(argv[2], "--system")) //if it is adduser with more than two names
    {
        printf("%s\n", "adduser: Only one or two names allowed.");
        return 0;
    }
    else if (argc == 3 && !strcmp(argv[1], "adduser") && !strcmp(argv[2], "--system")) //if it is adduser --system only (without any name)
    {
        printf("%s\n", "adduser: Only one or two names allowed.");
        return 0;
    }
    else if (argc > 4 && !strcmp(argv[1], "adduser") && !strcmp(argv[2], "--system")) //if it is adduser --system with more than one name
    {
        printf("%s\n", "adduser: Specify only one name in this mode.");
        return 0;
    }
    else if (strcmp(argv[1], "adduser")) //if 2nd argument is other than 'adduser'
    {
        printf("%s: %s\n", argv[0], "Expecting adduser command.");
        return 0;
    }

    extractInfo(envp, SHELL, "SHELL", sizeof("SHELL") - 1); //getting shell info. for passwd file
    extractInfo(envp, PWD, "PWD", sizeof("PWD") - 1);       //getting pwd info. for default path value when PFILE not available
    /*
    **************************************************
    * THIS IS YOUR PFILE ADDRESS LOCATION EXTRACTION *
    * PART FROM ENVIRONMENT VARIABLE. IF NO ADDRESS  *      
    * IS FOUND THEN IT WILL USE PWD AS LOCATION FOR  *
    * PASSWD AND SHADOW                              * 
    **************************************************
    */
    extractInfo(envp, PFILE, "PFILE", sizeof("PFILE") - 1);

    //printf("\nSHELL: %s\nPWD: %s\nPFILE: %s\n", SHELL, PWD, PFILE);

    if (PFILE[0] == 0)
    {
        strcpy(P_PATH, PWD); //path for PASSWD file (will be set auomatically when PFILE or PWD available)
        strcpy(S_PATH, PWD); //path for SHADOW file (will be set auomatically when PFILE or PWD available)
        strcpy(PATH, PWD);   // SET PATH TO HOME DIRECTORY (CURRENTLY USING PFILE OR PWD)
        strcat(P_PATH, "/passwd");
        strcat(S_PATH, "/shadow");
    }
    else
    {
        strcpy(P_PATH, PFILE); //----------do----------------
        strcpy(S_PATH, PFILE); //-------------do-------------------
        strcpy(PATH, PFILE);   //----------------do-----------------
        strcat(P_PATH, "/passwd");
        strcat(S_PATH, "/shadow");
    }
    strcat(PATH, "/");

    passwd = fopen(P_PATH, "a");
    shadow = fopen(S_PATH, "a");

    UID = rand() % 1000 + 1000; //Change this line with different algorithm for unique UID (currently generating random values in range of 1000)

    GID = rand() % 1000 + 1000; //Change this line  with different algorithm for unique GID (currently generating random values in range of 1000)

    encrypt = '!'; //use some encryption algorithm for the encryption in shadow file (currently displaying '!' for locked) use format $id$salt$hashed

    min = 0; //some default values for shadow file

    max = 99999; //some default values for shadow file

    warn = 7; //some default values for shadow file

    inactive = 0; //some default values for shadow file

    expire = 0; //some default values for shadow file

    if (argc == 3 && !strcmp(argv[1], "adduser") && strcmp(argv[2], "--system")) //if no. of arguments are 3 with adduser for normal user
    {
        printf("\nAdding user `%s' ...", argv[2]);
        printf("\nAdding new group `%s' (%d) ...", argv[2], GID);
        printf("\nAdding new user `%s' (%d) with group `%s' ...", argv[2], UID, argv[2]);
        strcat(PATH, argv[2]);
        printf("\nCreating home directory `%s' ...", PATH);
        if (stat(PATH, &st) == -1)
        {
            flag = mkdir(PATH, 0700); //creating a directory
            if (flag)
            {
                printf("\nError in Directory Creation.\n");
                return 1;
            }
        }
        printf("\nCopying files from `/etc/skel' ..."); //Set /etc/skel to some value (currently using it, as it is)

        c = 'y';
        while (c == 'y' || c == 'Y')
        {
            flag = 0;

            printf("\nNew password: ");
            fflush(stdin);

            fgets(password, sizeof(password), stdin);

            printf("\nRetype new password: ");
            c = 0;

            fgets(password2, sizeof(password2), stdin);

            if (strcmp(password2, password))
            {
                printf("\nSorry, passwords do not match.\npasswd: Authentication token manipulation error\npasswd: password unchanged");
                printf("\nTry again? [y/N] ");
                scanf("%c", &c);
            }
            while ((getchar()) != '\n')
                ;
        }

        c = 'n';
        while (c == 'n' || c == 'N')
        {
            printf("\nChanging the user information for %s", argv[2]);
            printf("\nEnter the new value, or press ENTER for the default");

            printf("\nFull Name []:");
            fscanf(stdin, "%s", fullname);
            printf("\nRoom Number []:");
            fscanf(stdin, "%s", roomnumber);
            printf("\nWork Phone []:");
            fscanf(stdin, "%s", workphone);
            printf("\nHome Phone []:");
            fscanf(stdin, "%s", homephone);
            printf("\nOther []:");
            fscanf(stdin, "%s", other);

            printf("\nIs the information correct? [Y/n] ");
            scanf("%c", &c);

            while ((getchar()) != '\n')
                ;
        }

        fprintf(passwd, "%s:x:%d:%d:%s,%s,%s,%s,%s:%s:%s\n", argv[2], UID, GID, fullname, roomnumber, workphone, homephone, other, PATH, SHELL);
        fprintf(shadow, "%s:%c:%ld:%d:%d:%d:%d:%d\n", argv[2], encrypt, seconds, min, max, warn, inactive, expire);
    }
    else if (argc == 4 && !strcmp(argv[1], "adduser") && strcmp(argv[2], "--system")) //for addser with two names
    {
        printf("\nAdding user `%s' to group `%s' ...", argv[2], argv[3]);
        printf("\nAdding user %s to group %s", argv[2], argv[3]);
        printf("\nDone.\n");
        fprintf(passwd, "%s:x:%d:%d::%s:%s\n", argv[3], UID, GID, PATH, "/usr/sbin/nologin");
        fprintf(shadow, "%s:%c:%ld:%d:%d:%d:%d:%d\n", argv[3], '*', seconds, min, max, warn, inactive, expire);
        //one more entry will be there in group file, since your query mentioned to work with passwd and shadow
        //I ignored the group file
    }
    else if (argc == 4 && !strcmp(argv[1], "adduser") && !strcmp(argv[2], "--system")) //for addser in system mode
    {
        printf("\nAdding system user '%s' (UID %d) ...", argv[3], UID);
        printf("\nAdding new user '%s' (UID %d) with group `nogroup' ...", argv[3], UID);
        strcat(PATH, argv[2]);
        printf("\nCreating home directory '%s' ...\n", PATH);
        if (stat(PATH, &st) == -1)
        {
            flag = mkdir(PATH, 0700);
            if (flag)
            {
                printf("\nError in Directory Creation.\n");
                return 1;
            }
        }

        fprintf(passwd, "%s:x:%d:%d::%s:%s\n", argv[2], UID, GID, PATH, "/usr/sbin/nologin");
        fprintf(shadow, "%s:%c:%ld:%d:%d:%d:%d:%d\n", argv[2], '*', seconds, min, max, warn, inactive, expire);
    }

    fclose(passwd);
    fclose(shadow);
    return 0;
}