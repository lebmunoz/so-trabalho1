// gcc -Wall -o jogo main.c -lcurses -lpthread

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
#define TK_CAPTURED 0
#define TK_FREE 1
#define GM_RUNNING 0
#define GM_STOP 1

void *handle_tokens(void *arg);
void *handle_cursor(void *arg);
void *handle_game(void *arg);
void move_token(int token);
void draw_board(void);
bool token_has_collision(int i);
void capture_token(int t);
int get_token_status(int t);
void set_game_status(int s);
int get_game_status();
sem_t mutex;

int difficulty = EASY;
char board[LINES][COLS];
int token_status[NUM_TOKENS];
int game_status = GM_RUNNING;

typedef struct CoordStruct {
    int x;
    int y;
} coord_type;

coord_type cursor, coord_tokens[NUM_TOKENS];

int main(void) {
    int game;
    int csr;
    int res;
    pthread_t game_thread;
    pthread_t cursor_thread;
    pthread_t a_thread[NUM_TOKENS];
    void *thread_result;
    void *game_thread_result;
    void *cursor_thread_result;
    int tk;
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

    game = pthread_create(&(game_thread), NULL, handle_game, NULL);
    if (game != 0) {
        perror("Criacao de Thread falhou");
        exit(EXIT_FAILURE);
    }

    csr = pthread_create(&(cursor_thread), NULL, handle_cursor, NULL);
    if (csr != 0) {
        perror("Criacao de Thread falhou");
        exit(EXIT_FAILURE);
    }

    for (tk = 0; tk < NUM_TOKENS; tk++) {
        token_status[tk] = TK_FREE;
        res = pthread_create(&(a_thread[tk]), NULL, handle_tokens, (void *)(intptr_t)tk);
        if (res != 0) {
            perror("Criacao de Thread falhou");
            exit(EXIT_FAILURE);
        }
    }
    
    for (tk = NUM_TOKENS - 1; tk >= 0; tk--) {
        res = pthread_join(a_thread[tk], &thread_result);
        if (res != 0) {
            perror("Thread falhou no join");
        }
    }
    game = pthread_join(game_thread, &game_thread_result);
    if (game != 0) {
        perror("Thread falhou no join");
    }
    csr = pthread_join(cursor_thread, &cursor_thread_result);
    if (csr != 0) {
        perror("Thread falhou no join");
    }
    printf("Comeu todas as threads!\n");
    exit(EXIT_SUCCESS);
}

bool token_has_collision(int i) {
    return cursor.x == coord_tokens[i].x && cursor.y == coord_tokens[i].y;
}

void move_token(int token) {
    int i = token, new_x, new_y;

    /* determina novas posicoes (coordenadas) do token no tabuleiro (matriz) */

    do {
        new_x = rand()%(COLS);
        new_y = rand()%(LINES);
    } while ((board[new_x][new_y] != 0) || ((new_x == cursor.x) && (new_y == cursor.y)));

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

void capture_token(int t) {
//    token_status[t] = TK_CAPTURED;
}

int get_token_status(int t) {
    return token_status[t];
}

void set_game_status(int s) {
    game_status = s;
}
int get_game_status(){
    return game_status;
}

void *handle_tokens(void *arg) {
    int my_number = (intptr_t) arg;
    do {
        move_token(my_number); /* move os tokens aleatoriamente */
        board_refresh(); /* atualiza token no tabuleiro */
        sleep(difficulty);
    } while (get_token_status(my_number) != TK_CAPTURED);

    printf("Comeu a thread %d\n", my_number);
    pthread_exit(NULL);
}

void *handle_cursor(void *arg) {
    int ch;

    do {
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
            case 'q':
            case 'Q':
                set_game_status(GM_STOP);
        }
    } while (get_game_status() == GM_RUNNING);
    endwin();
    pthread_exit(NULL);
}

void *handle_game(void *arg) {
    do {
        for (int i = 0; i < NUM_TOKENS; ++i) {
            if (token_has_collision(i)) {
                capture_token(i);
            }
        }
    } while (get_game_status() == GM_RUNNING);
    pthread_exit(NULL);
}