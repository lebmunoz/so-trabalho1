#include <curses.h>
#include <stdlib.h>
#include <time.h>

#define EMPTY     ' '
#define CURSOR_PAIR   1
#define TOKEN_PAIR    2
#define EMPTY_PAIR    3
#define LINES 11
#define COLS 11
#define TOKENS 5

int is_move_okay(int y, int x);
void draw_board(void);
void move_token(int token);
void move_tokens();
void board_refresh(void);

char board[LINES][COLS];

typedef struct CoordStruct {
  int x;
  int y;
} coord_type;
 
coord_type cursor, coord_tokens[TOKENS];

int main(void) {
  int ch;
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
  
  do {   
    move_tokens(); /* move os tokens aleatoriamente */
    board_refresh(); /* redesenha tabuleiro */
    
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
  exit(0);
}

void move_tokens() {
  int i;
  for (i = 0; i < TOKENS; i++)
    move_token(i);
}

void board_refresh(void) {
  int x, y, i;

  /* redesenha tabuleiro "limpo" */

  for (x = 0; x < COLS; x++) 
    for (y = 0; y < LINES; y++){
      attron(COLOR_PAIR(EMPTY_PAIR));
      mvaddch(y, x, EMPTY);
      attroff(COLOR_PAIR(EMPTY_PAIR));
  }

  /* poe os tokens no tabuleiro */

  for (i = 0; i < TOKENS; i++) {
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
