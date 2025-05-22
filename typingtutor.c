
//header files
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<ctype.h>

//global declarations

#define max_para_length 200
#define max_file_line_length 200
#define max_attempts 10
#define max_leaderboard_entries 100

// Define named constants for difficulty levels
#define EASY_SPEED 5
#define MEDIUM_SPEED 8
#define HARD_SPEED 12
#define EASY_MEDIUM_SPEED 8
#define MEDIUM_HARD_SPEED 12
#define HARD_MAX_SPEED 16

//structure to store user profile
typedef struct {
    char username[50];
    double bestSpeed;
    double bestAccuracy;
    double totalSpeed;
    double totalAccuracy;
    int totalAttempts;
} UserProfile;

// structure to store difficulty
typedef struct
{
    int easy;
    int medium;
    int hard;
} Difficulty;

//structure to store typing statistics
typedef struct  {
    double typingSpeed;
    double accuracy;
    int wrongChars;
    char paragraph[max_para_length];
    double wordsPerMinute;

} TypingStats;

typedef struct {
    char username[50];
    double typingSpeed;
    double wordsPerMinute;
    double accuracy;
    char difficulty[10];
} LeaderboardEntry;


// Function to load the user profile from a file
void loadUserProfile(UserProfile* profile) {
    printf("Enter your username: ");
    fgets(profile->username, sizeof(profile->username), stdin);
    size_t len = strlen(profile->username);
    if (len > 0 && profile->username[len - 1] == '\n') {
        profile->username[len - 1] = '\0';
    }
    char filename[100];
    snprintf(filename, sizeof(filename), "%s_profile.txt", profile->username);
    FILE* f = fopen(filename, "r");
    if (f != NULL) {
        if (fscanf(f, "%lf %lf %lf %lf %d",
                   &profile->bestSpeed, &profile->bestAccuracy,
                   &profile->totalSpeed, &profile->totalAccuracy,
                   &profile->totalAttempts) != 5) {
            profile->bestSpeed = 0;
            profile->bestAccuracy = 0;
            profile->totalSpeed = 0;
            profile->totalAccuracy = 0;
            profile->totalAttempts = 0;
        }
        fclose(f);
    }
    else
    {
        if (f)
            fclose(f);
        profile->bestSpeed = profile->bestAccuracy = profile->totalSpeed = profile->totalAccuracy = 0;
        profile->totalAttempts = 0;
    }
}

// Function to update and save user profile after each attempt
void updateUserProfile(UserProfile* profile, TypingStats* currentAttempt) {
    if (currentAttempt->typingSpeed > profile->bestSpeed) {
        profile->bestSpeed = currentAttempt->typingSpeed;
    if (currentAttempt->accuracy > profile->bestAccuracy)
        profile->bestAccuracy = currentAttempt->accuracy;
    }
    profile->totalSpeed += currentAttempt->typingSpeed;
    profile->totalAccuracy += currentAttempt->accuracy;
    profile->totalAttempts++;
    char filename[100];
    snprintf(filename, sizeof(filename), "%s_profile.txt", profile->username);
    FILE* f = fopen(filename, "w");

    if (f != NULL) {
        fprintf(f, "%.2lf %.2lf %.2lf %.2lf %d\n",
                profile->bestSpeed, profile->bestAccuracy,
                profile->totalSpeed, profile->totalAccuracy,
                profile->totalAttempts);
        fclose(f);
    } else {
        perror("Error saving user profile");
    }
}

// Function to display user summary
void displayUserSummary(const UserProfile* profile) {

    printf("\nUser Summary for %s:\n", profile->username);
    printf("--------------------------------------------------------\n");
    printf("Best Typing Speed: %.2f cpm\n", profile->bestSpeed);  
    printf("Best Accuracy: %.2f%%\n", profile->bestAccuracy);
    if (profile->totalAttempts > 0)
    {
        printf("Average Typing Speed: %.2f cpm\n", profile->totalSpeed / profile->totalAttempts);
        printf("Average Accuracy: %.2f%%\n", profile->totalAccuracy / profile->totalAttempts);
    }
    printf("Total Attempts: %d\n", profile->totalAttempts);
    printf("--------------------------------------------------------\n");
}

// Function to get a random paragraph from file
char* getRandomParagraph(FILE* file) {
    char* paragraphs[100];
    int count = 0;
    char line[max_file_line_length];

    while (fgets(line, sizeof(line), file) != NULL && count < 100) {
        size_t len = strlen(line);
        if (len > 1 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (strlen(line) > 0) {
            paragraphs[count++] = strdup(line);
        }
    }

    if (count == 0) {
        fprintf(stderr, "No paragraphs found in the file.\n");
        exit(EXIT_FAILURE);
    }

    int randomIndex = rand() % count;
    char* result = strdup(paragraphs[randomIndex]);

    for (int i = 0; i < count; ++i) {
        free(paragraphs[i]);
    }

    return result;
}

// Function to calculate and print typing statistics
void printTypingStats(double elapsedTime, const char* input, const char* correctText, TypingStats* stats) {
    int correctCount = 0, wrongCount = 0;
    int minLen = strlen(correctText) < strlen(input) ? strlen(correctText) : strlen(input);

    for (int i = 0; i < minLen; i++) {
        if (correctText[i] == input[i])
            correctCount++;
        else
            wrongCount++;
    }

    int totalCharacters = minLen > 0 ? minLen : 1;
    double accuracy = (double)correctCount / totalCharacters * 100;

    if (elapsedTime < 3) {
        printf("\n### Please do not paste or hit enter immediately ###\n\n");
        elapsedTime = 3;
    }

    double wpm = (totalCharacters / 5.0) / (elapsedTime / 60.0);
    double cpm = wpm * 5;

    stats->typingSpeed = cpm;
    stats->accuracy = accuracy;
    stats->wrongChars = wrongCount;
    stats->wordsPerMinute = wpm;

    strncpy(stats->paragraph, correctText, max_para_length);
}


// Display all previous attempts
void displayPreviousAttempts(TypingStats attempts[], int numAttempts) {
    printf("\nPrevious Attempts:\n");
    printf("---------------------------------------------------------------------\n");
   printf("| Attempt |  CPM   |  WPM   | Accuracy (%%) | Wrong Chars |\n");
printf("---------------------------------------------------------------------\n");

for (int i = 0; i < numAttempts; i++) {
    printf("|   %2d    | %6.2f | %6.2f |    %6.2f%%   |     %3d     |\n",
           i + 1, attempts[i].typingSpeed, attempts[i].wordsPerMinute, attempts[i].accuracy, attempts[i].wrongChars);
}


    printf("---------------------------------------------------------------------\n");
}

// Prompt user to choose difficulty
void promptDifficulty(Difficulty *difficulty, char *difficultyStr) {
    int choice;
    printf("Select difficulty level:\n1. Easy\n2. Medium\n3. Hard\n");

    while (1) {
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        if (choice >= 1 && choice <= 3) break;
        printf("Invalid choice. Please enter a number between 1 and 3.\n");
    }

    switch (choice) {
    case 1:
        *difficulty = (Difficulty){EASY_SPEED, EASY_MEDIUM_SPEED, MEDIUM_HARD_SPEED};
        strcpy(difficultyStr, "Easy");
        break;
    case 2:
        *difficulty = (Difficulty){EASY_MEDIUM_SPEED, MEDIUM_HARD_SPEED, HARD_MAX_SPEED};
        strcpy(difficultyStr, "Medium");
        break;
    case 3:
        *difficulty = (Difficulty){MEDIUM_HARD_SPEED, HARD_MAX_SPEED, HARD_SPEED + 4};
        strcpy(difficultyStr, "Hard");
        break;
    default:
        *difficulty = (Difficulty){EASY_SPEED, EASY_MEDIUM_SPEED, MEDIUM_HARD_SPEED};
        strcpy(difficultyStr, "Easy");
}

    
}

// Save leaderboard to file
void saveLeaderboard(LeaderboardEntry leaderboard[], int numEntries)
{
    FILE *file = fopen("leaderboard.txt", "w");
    if (!file)
    {
        perror("Error saving leaderboard");
        return;
    }

    for (int i = 0; i < numEntries; i++)
    {
        fprintf(file, "%s %.2f %.2f %.2f %s\n", leaderboard[i].username,
                leaderboard[i].typingSpeed,
                leaderboard[i].wordsPerMinute,
                leaderboard[i].accuracy,
                leaderboard[i].difficulty);
    }

    fclose(file);
}

void loadLeaderboard(LeaderboardEntry leaderboard[], int *numEntries) {
    FILE *file = fopen("leaderboard.txt", "r");
    if (!file) {
        *numEntries = 0;
        return;
    }

    *numEntries = 0;
    while (fscanf(file, "%s %lf %lf %lf %s",
                  leaderboard[*numEntries].username,
                  &leaderboard[*numEntries].typingSpeed,
                  &leaderboard[*numEntries].wordsPerMinute,
                  &leaderboard[*numEntries].accuracy,
                  leaderboard[*numEntries].difficulty) == 5) {
        (*numEntries)++;
        if (*numEntries >= max_leaderboard_entries) break;
    }

    fclose(file);
}


// Update leaderboard with current attempt
void updateLeaderboard(UserProfile *profile, TypingStats *currentAttempt, const char *difficulty)
{
    LeaderboardEntry leaderboard[max_leaderboard_entries];
    int numEntries;

    // Load the existing leaderboard
    loadLeaderboard(leaderboard, &numEntries);

    // Create a new entry for the current attempt
    LeaderboardEntry newEntry;
    strncpy(newEntry.username, profile->username, sizeof(newEntry.username));
    newEntry.typingSpeed = currentAttempt->typingSpeed;
    newEntry.wordsPerMinute = currentAttempt->wordsPerMinute;
    newEntry.accuracy = currentAttempt->accuracy;
    strncpy(newEntry.difficulty, difficulty, sizeof(newEntry.difficulty));

    // Add the new entry to the leaderboard
    if (numEntries < max_leaderboard_entries)
    {
        leaderboard[numEntries++] = newEntry;
    }
    else
    {
        // If leaderboard is full, replace the worst entry if the current one is better
        int worstIndex = 0;
        for (int i = 1; i < numEntries; i++)
        {
            if (leaderboard[i].typingSpeed < leaderboard[worstIndex].typingSpeed &&
                strcmp(leaderboard[i].difficulty, difficulty) == 0)
            {
                worstIndex = i;
            }
        }

        if (newEntry.typingSpeed > leaderboard[worstIndex].typingSpeed ||
            numEntries < max_leaderboard_entries)
        {
            leaderboard[worstIndex] = newEntry;
        }
    }

    // Sort the leaderboard by typing speed (descending order) for each difficulty
    for (int i = 0; i < numEntries - 1; i++)
    {
        for (int j = i + 1; j < numEntries; j++)
        {
            if (strcmp(leaderboard[i].difficulty, leaderboard[j].difficulty) == 0 &&
                leaderboard[i].typingSpeed < leaderboard[j].typingSpeed)
            {
                LeaderboardEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }

    // Save the updated leaderboard
    saveLeaderboard(leaderboard, numEntries);
}

// Display leaderboard for a specific difficulty
void displayLeaderboard(const char *difficulty)
{
    LeaderboardEntry leaderboard[max_leaderboard_entries];
    int numEntries;

    // Load the leaderboard
    loadLeaderboard(leaderboard, &numEntries);

    printf("\nLeaderboard for %s Difficulty:\n", difficulty);
    printf("-------------------------------------------------------------\n");
    printf("| Rank | Username       | CPM    | WPM    | Accuracy (%%) |\n");
    printf("-------------------------------------------------------------\n");

    int rank = 1;
    for (int i = 0; i < numEntries; i++)
    {
        if (strcmp(leaderboard[i].difficulty, difficulty) == 0)
        {
            printf("| %4d | %-14s | %6.2f | %6.2f | %10.2f |\n",
                   rank++, leaderboard[i].username,
                   leaderboard[i].typingSpeed,
                   leaderboard[i].wordsPerMinute,
                   leaderboard[i].accuracy);

            if (rank > 10)
                break; // Only show top 10 entries
        }
    }

    if (rank == 1)
    {
        printf("|      No entries for this difficulty level yet          |\n");
    }

    printf("-------------------------------------------------------------\n");
}
void showLeaderboardMenu() {
    int choice;
    char difficulty[10];

    while (1) {
        printf("\nLeaderboard Menu:\n");
        printf("1. Easy\n2. Medium\n3. Hard\n4. Back to main menu\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // clear input buffer
            continue;
        }
        while (getchar() != '\n'); // clear input buffer
        
        switch (choice) {
            case 1:
                strcpy(difficulty, "Easy");
                displayLeaderboard(difficulty);
                break;
            case 2:
                strcpy(difficulty, "Medium");
                displayLeaderboard(difficulty);
                break;
            case 3:
                strcpy(difficulty, "Hard");
                displayLeaderboard(difficulty);
                break;
            case 4:
                return; // exit menu
            default:
                printf("Invalid choice. Please select 1-4.\n");
        }
    }
}


// Main typing loop
int main() {
    char selectedDifficulty[10];

    srand((unsigned int)time(NULL));
    UserProfile user;
    TypingStats attempts[max_attempts];
    Difficulty difficulty;

    loadUserProfile(&user);
    promptDifficulty(&difficulty, selectedDifficulty);


    int numAttempts = 0;
    char tryAgain;

    do {
        FILE* paraFile = fopen("paragraphs.txt", "r");
        if (!paraFile) {
            perror("Error opening paragraphs file");
            return 1;
        }

        TypingStats currentStats;

        char* paragraph = getRandomParagraph(paraFile);
        fclose(paraFile);
        printf("\nType the following paragraph:\n\n%s\n", paragraph);
        printf("\nPress Enter when you finish typing.\n\n");

        char inputText[max_para_length];
        int idx = 0;
        char ch;
        clock_t start = clock();
        while (idx < max_para_length - 1) {
            ch = getchar();
            if (ch == '\n') break;
            inputText[idx++] = ch;

        }
        inputText[idx] = '\0';
        clock_t end = clock();

        double elapsedTime = (double)(end - start) / CLOCKS_PER_SEC;

        printTypingStats(elapsedTime, inputText, paragraph, &currentStats);
        attempts[numAttempts++] = currentStats;
        updateUserProfile(&user, &currentStats);
        updateLeaderboard(&user, &currentStats, selectedDifficulty);


        free(paragraph);

        printf("\nTyping Speed: %.2f cpm\n", currentStats.typingSpeed);
        printf("Accuracy: %.2f%%\n", currentStats.accuracy);
        printf("Wrong Characters: %d\n", currentStats.wrongChars);

        if (numAttempts < max_attempts) {
            printf("\nDo you want to try again? (y/n): ");
            scanf(" %c", &tryAgain);
            while (getchar() != '\n');
        } else {
            printf("\nYou've reached the maximum number of attempts.\n");
            tryAgain = 'n';
        }

    } while (tolower(tryAgain) == 'y');


    displayUserSummary(&user);
    displayPreviousAttempts(attempts, numAttempts);
    char menuChoice;

do {
    printf("\nWhat would you like to do next?\n");
    printf("1. View Leaderboard\n");
    printf("2. View User Summary\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");

    menuChoice = getchar();
    while (getchar() != '\n'); // clear input buffer

    switch (menuChoice) {
        case '1':
            showLeaderboardMenu();
            break;
        case '2':
            displayUserSummary(&user);
            break;
        case '3':
            printf("Exiting program. Goodbye!\n");
            break;
        default:
            printf("Invalid choice, please enter 1, 2 or 3.\n");
    }
} while (menuChoice != '3');


    return 0;
}

