#include <curses.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_TOKENS 5

#define EMPTY     ' '
#define CURSOR_PAIR   1
#define TOKEN_PAIR    2
#define EMPTY_PAIR    3
#define LINES 11
#define COLS 11
#define EASY 4
#define HARD 1

void *handle_tokens(void *arg);
void *handle_cursor(void *arg);
void move_token(int token);
void draw_board(void);
void clear_board(void);
void token_refresh(void);
bool token_has_collision(int i);
void cursor_refresh(void);
sem_t mutex;

int difficulty = EASY;
char board[LINES][COLS];

typedef struct CoordStruct {
    int x;
    int y;
} coord_type;

coord_type cursor, coord_tokens[NUM_TOKENS];

int main(void) {
    int game;
    int res;
    pthread_t game_thread;
    pthread_t a_thread[NUM_TOKENS];
    void *thread_result;
    int tokens;
    sem_init(&mutex, 0, 1);

    srand(time(NULL));  /* inicializa gerador de numeros aleatorios */

    /* inicializa curses */

    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    /* inicializa colors */

    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal nao suporta cor\n");
        exit(1);
    }

    start_color();

    /* inicializa pares caracter-fundo do caracter */

    init_pair(CURSOR_PAIR, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(TOKEN_PAIR, COLOR_RED, COLOR_RED);
    init_pair(EMPTY_PAIR, COLOR_BLUE, COLOR_BLUE);
    clear();

    draw_board(); /* inicializa tabuleiro */

    game = pthread_create(&(game_thread), NULL, handle_cursor, NULL);
    if (game != 0) {
        perror("Criacao de Thread falhou");
        exit(EXIT_FAILURE);
    }

    for(tokens = 0; tokens < NUM_TOKENS; tokens++) {
        res = pthread_create(&(a_thread[tokens]), NULL, handle_tokens, (void *)(intptr_t)tokens);
        if (res != 0) {
            perror("Criacao de Thread falhou");
            exit(EXIT_FAILURE);
        }
    }
//    printf("Esperando por thread finalizar...\n");
    for(tokens = NUM_TOKENS - 1; tokens >= 0; tokens--) {
        res = pthread_join(a_thread[tokens], &thread_result);
        if (res != 0) {
            perror("Thread falhou no join");
        }
    }
    printf("Comeu todas as threads!\n");
    exit(EXIT_SUCCESS);
    // res = pthread_join(game_thread, &thread_result);
    // if (res != 0) {
    //     perror("Thread falhou no join");
    // }
    // printf("Todas terminaram\n");
}

void clear_board(void) {
    int x, y;

    /* redesenha tabuleiro "limpo" */

    for (x = 0; x < COLS; x++)
        for (y = 0; y < LINES; y++){
            attron(COLOR_PAIR(EMPTY_PAIR));
            mvaddch(y, x, EMPTY);
            attroff(COLOR_PAIR(EMPTY_PAIR));
        }
}

void token_refresh(void) {
    int i;
    clear_board();

    /* poe os tokens no tabuleiro */

    for (i = 0; i < NUM_TOKENS; i++) {
        attron(COLOR_PAIR(TOKEN_PAIR));
        mvaddch(coord_tokens[i].y, coord_tokens[i].x, EMPTY);
        attroff(COLOR_PAIR(TOKEN_PAIR));
    }
}

bool token_has_collision(int i) {
    return cursor.x == coord_tokens[i].x && cursor.y == coord_tokens[i].y;
}

void cursor_refresh(void) {
    int x, y;

    clear_board();

    /* poe o cursor no tabuleiro */

    move(y, x);
    refresh();
    attron(COLOR_PAIR(CURSOR_PAIR));
    mvaddch(cursor.y, cursor.x, EMPTY);
    attroff(COLOR_PAIR(CURSOR_PAIR));
}

void move_token(int token) {
    int i = token, new_x, new_y;

    /* determina novas posicoes (coordenadas) do token no tabuleiro (matriz) */

    do{
        new_x = rand()%(COLS);
        new_y = rand()%(LINES);
    } while((board[new_x][new_y] != 0) || ((new_x == cursor.x) && (new_y == cursor.y)));

    /* retira token da posicao antiga  */

    board[coord_tokens[i].x][coord_tokens[i].y] = 0;
    board[new_x][new_y] = token;

    /* coloca token na nova posicao */
    coord_tokens[i].x = new_x;
    coord_tokens[i].y = new_y;
}

void draw_board(void) {
    int x, y;

    /* limpa matriz que representa o tabuleiro */
    for (x = 0; x < COLS; x++)
        for (y = 0; y < LINES; y++)
            board[x][y] = 0;
}

void board_refresh(void) {
  int x, y, i;

  /* redesenha tabuleiro "limpo" */
  sem_wait(&mutex);
  for (x = 0; x < COLS; x++) 
    for (y = 0; y < LINES; y++){
      attron(COLOR_PAIR(EMPTY_PAIR));
      mvaddch(y, x, EMPTY);
      attroff(COLOR_PAIR(EMPTY_PAIR));
  }

  /* poe os tokens no tabuleiro */

  for (i = 0; i < NUM_TOKENS; i++) {
    attron(COLOR_PAIR(TOKEN_PAIR));
    mvaddch(coord_tokens[i].y, coord_tokens[i].x, EMPTY);
    attroff(COLOR_PAIR(TOKEN_PAIR));
  }
  /* poe o cursor no tabuleiro */

  move(y, x);
  refresh();
  attron(COLOR_PAIR(CURSOR_PAIR));
  mvaddch(cursor.y, cursor.x, EMPTY);
  attroff(COLOR_PAIR(CURSOR_PAIR));
  sem_post(&mutex);
}

void *handle_tokens(void *arg) {
    int my_number = (intptr_t) arg;

    do {
        //sem_wait(&mutex);
        move_token(my_number); /* move os tokens aleatoriamente */
        board_refresh(); /* atualiza token no tabuleiro */
        sleep(difficulty);
        //sem_post(&mutex);
    }
    while(!token_has_collision(my_number));

    printf("Comeu a thread %d\n", my_number);
    pthread_exit(NULL);
}

void *handle_cursor(void *arg) {
//    int my_number = *(int *)arg;
    int ch;

    do {
        // cursor_refresh(); /* atualiza cursor no tabuleiro */

        board_refresh();
        
        ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                if ((cursor.y > 0)) {
                    cursor.y = cursor.y - 1;
                }
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                if ((cursor.y < LINES - 1)) {
                    cursor.y = cursor.y + 1;
                }
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                if ((cursor.x > 0)) {
                    cursor.x = cursor.x - 1;
                }
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if ((cursor.x < COLS - 1)) {
                    cursor.x = cursor.x + 1;
                }
                break;
        }
    }while ((ch != 'q') && (ch != 'Q'));
    endwin();

//    printf("Fim de jogo %d\n", my_number);
    pthread_exit(NULL);
}