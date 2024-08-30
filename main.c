#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 操作方法
char* howToPlayText = 
"\
----操作方法----\n\
a, h: 左移動\n\
d, l: 右移動\n\
s, j: 下移動\n\
S, J: 一番下まで移動\n\
r, i: 回転\n\
q, ctrl-c: 終了\n\
5操作に1回自動で下移動\n\
";

// ミノ
#define MINO_HEIGHT 4
#define MINO_WIDTH 4
#define MINO_SIZE (MINO_HEIGHT * MINO_WIDTH)
#define MINO_TYPE_NUM 7

typedef char mino_type_t;
mino_type_t mino_types[MINO_TYPE_NUM] = {'I', 'O', 'S', 'Z', 'L', 'J', 'T'};

typedef struct _mino_t {
  mino_type_t type;
  int x;
  int y;
  int rot;
} mino_t;

mino_t *gMovingMino;
mino_t *gNextMino;

int gIMinoShape[MINO_SIZE] = {
  0, 0, 0, 0,
  1, 1, 1, 1,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int gOMinoShape[MINO_SIZE] = {
  1, 1, 0, 0,
  1, 1, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int gSMinoShape[MINO_SIZE] = {
  0, 1, 1, 0,
  1, 1, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int gZMinoShape[MINO_SIZE] = {
  1, 1, 0, 0,
  0, 1, 1, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int gJMinoShape[MINO_SIZE] = {
  1, 0, 0, 0,
  1, 1, 1, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int gLMinoShape[MINO_SIZE] = {
  0, 0, 1, 0,
  1, 1, 1, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int gTMinoShape[MINO_SIZE] = {
  0, 1, 0, 0,
  1, 1, 1, 0,
  0, 0, 0, 0,
  0, 0, 0, 0
};

#define SCRN_HEIGHT 23
#define SCRN_WIDTH 30
#define SCRN_SIZE (SCRN_HEIGHT * SCRN_WIDTH)
char gScrn[SCRN_SIZE] = {};

#define FIELD_HEIGHT 20
#define FIELD_WIDTH 10
#define FIELD_SIZE (FIELD_HEIGHT * FIELD_WIDTH)
#define FIELD_OFFSET_X 1
#define FIELD_OFFSET_Y 1
mino_type_t gField[FIELD_SIZE] = {};

#define NEXT_FIELD_HEIGHT (MINO_HEIGHT + 2)
#define NEXT_FIELD_WIDTH (MINO_WIDTH + 2)
#define NEXT_FIELD_SIZE (NEXT_FIELD_HEIGHT * NEXT_FIELD_WIDTH)
#define NEXT_FIELD_OFFSET_X (FIELD_OFFSET_X + FIELD_WIDTH + 2)
#define NEXT_FIELD_OFFSET_Y FIELD_OFFSET_Y
mino_type_t gNextField[NEXT_FIELD_SIZE] = {};

void init();
void print_scrn();
void create_next_mino();
void create_new_mino();
void print_mino(mino_type_t);
int *mino_to_shape(mino_t *);
int *rotate_mino(int *, int, float);
int mino_to_field(mino_t *);
int mino_to_next_field(mino_t *);
int mino_to_scrn(mino_t *);
int move_mino(char);
void delite_line(int);
int is_collision(mino_t *);
int is_completed_line(int);
int is_dead();

void init() {
  srand(time(NULL));
  for (int y = 0; y < SCRN_HEIGHT; y++) {
    for (int x = 0; x < SCRN_WIDTH; x++) {
      if ((FIELD_OFFSET_X <= x && x <= FIELD_OFFSET_X + FIELD_WIDTH) &&
          (y == FIELD_OFFSET_Y + FIELD_HEIGHT))
        gScrn[SCRN_WIDTH * y + x] = 'b';
      else if ((x == FIELD_OFFSET_X - 1 || x == FIELD_OFFSET_X + FIELD_WIDTH) &&
               (FIELD_OFFSET_Y <= y && y <= FIELD_OFFSET_Y + FIELD_HEIGHT))
        gScrn[SCRN_WIDTH * y + x] = 'b';
      else if ((NEXT_FIELD_OFFSET_X <= x &&
                x <= NEXT_FIELD_OFFSET_X + NEXT_FIELD_WIDTH + 1) &&
               (y == NEXT_FIELD_OFFSET_Y ||
                y == NEXT_FIELD_OFFSET_Y + NEXT_FIELD_HEIGHT + 1))
        gScrn[SCRN_WIDTH * y + x] = 'b';
      else if ((x == NEXT_FIELD_OFFSET_X ||
                x == NEXT_FIELD_OFFSET_X + NEXT_FIELD_WIDTH + 1) &&
               (NEXT_FIELD_OFFSET_Y <= y &&
                y <= NEXT_FIELD_OFFSET_Y + NEXT_FIELD_HEIGHT + 1))
        gScrn[SCRN_WIDTH * y + x] = 'b';
    }
  }
  create_next_mino();
}

void print_scrn() {
  mino_to_next_field(gNextMino);
  mino_to_field(gMovingMino);
  printf("\n");
  for (int y = 0; y < SCRN_HEIGHT; y++) {
    for (int x = 0; x < SCRN_WIDTH; x++) {
      if (gScrn[SCRN_WIDTH * y + x])
        print_mino(gScrn[SCRN_WIDTH * y + x]);
      else if ((0 <= y - FIELD_OFFSET_Y && y - FIELD_OFFSET_Y < FIELD_HEIGHT) &&
               (0 <= x - FIELD_OFFSET_X && x - FIELD_OFFSET_X < FIELD_WIDTH) &&
               gField[FIELD_WIDTH * (y - FIELD_OFFSET_Y) + x - FIELD_OFFSET_X])
        print_mino(
            gField[FIELD_WIDTH * (y - FIELD_OFFSET_Y) + x - FIELD_OFFSET_X]);
      else if ((0 <= y - NEXT_FIELD_OFFSET_Y - 1 &&
                y - NEXT_FIELD_OFFSET_Y - 1 < NEXT_FIELD_HEIGHT) &&
               (0 <= x - NEXT_FIELD_OFFSET_X - 1 &&
                x - NEXT_FIELD_OFFSET_X - 1 < NEXT_FIELD_WIDTH) &&
               gNextField[NEXT_FIELD_WIDTH * (y - NEXT_FIELD_OFFSET_Y - 1) + x -
                          NEXT_FIELD_OFFSET_X - 1]) {
        print_mino(gNextField[NEXT_FIELD_WIDTH * (y - NEXT_FIELD_OFFSET_Y - 1) +
                              x - NEXT_FIELD_OFFSET_X - 1]);
      }

      else
        printf("　");
    }
    printf("\n");
  }
  printf("\n%s\n", howToPlayText);
  for (int i = 0; i < FIELD_SIZE; i++)
    gField[i] = 0;
  for (int i = 0; i < NEXT_FIELD_SIZE; i++)
    gNextField[i] = 0;
}

void create_next_mino() {
  gNextMino = (mino_t *)malloc(sizeof(mino_t));
  gNextMino->type = mino_types[rand() % MINO_TYPE_NUM];
  gNextMino->x = FIELD_WIDTH / 2 - 2;
  gNextMino->y = 0;
  gNextMino->rot = rand() % 4;
}

void create_new_mino() {
  gMovingMino = gNextMino;
  create_next_mino();
}

void print_mino(mino_type_t type) {
  if (type == 'b')  printf("🔳");
  else {
    if(type == 'I')       printf("\x1b[46m");
    else if(type == 'O')  printf("\x1b[43m");
    else if(type == 'S')  printf("\x1b[41m");
    else if(type == 'Z')  printf("\x1b[42m");
    else if(type == 'L')  printf("\x1b[44m");
    else if(type == 'J')  printf("\x1b[47m");
    else if(type == 'T')  printf("\x1b[45m");
    printf("🔲");
    printf("\x1b[49m");
  }
}

int *mino_to_shape(mino_t *mino) {
  if (mino->type == 'I') {
    return rotate_mino(gIMinoShape, mino->rot, 1.5);
  } else if (mino->type == 'O') {
    return gOMinoShape;
  } else if (mino->type == 'S') {
    return rotate_mino(gSMinoShape, mino->rot, 1);
  } else if (mino->type == 'Z') {
    return rotate_mino(gZMinoShape, mino->rot, 1);
  } else if (mino->type == 'L') {
    return rotate_mino(gLMinoShape, mino->rot, 1);
  } else if (mino->type == 'J') {
    return rotate_mino(gJMinoShape, mino->rot, 1);
  } else if (mino->type == 'T') {
    return rotate_mino(gTMinoShape, mino->rot, 1);
  }
}

int *rotate_mino(int *shape, int rot, float rotCenter) {
  if (!rot)
    return shape;
  int *rotShape = (int *)malloc(sizeof(int) * MINO_SIZE);
  int rotMat[2][2] = {{0, -1}, {1, 0}};
  for (int y = 0; y <= rotCenter * 2; y++) {
    for (int x = 0; x <= rotCenter * 2; x++) {
      int newX = x;
      int newY = y;
      for (int i = 0; i < rot; i++) {
        int bnx = newX, bny = newY;
        newX = bnx * rotMat[0][0] + bny * rotMat[0][1] + rotCenter -
               rotCenter * rotMat[0][0] - rotCenter * rotMat[0][1];
        newY = bnx * rotMat[1][0] + bny * rotMat[1][1] + rotCenter -
               rotCenter * rotMat[1][0] - rotCenter * rotMat[1][1];
      }
      rotShape[MINO_WIDTH * newY + newX] = shape[MINO_WIDTH * y + x];
    }
  }
  return rotShape;
}

int mino_to_field(mino_t *mino) {
  int *shape = mino_to_shape(mino);
  for (int y = 0; y < MINO_HEIGHT; y++) {
    for (int x = 0; x < MINO_WIDTH; x++) {
      if ((0 <= y + mino->y && y + mino->y < FIELD_HEIGHT) &&
          (0 <= x + mino->x && x + mino->x < FIELD_WIDTH) &&
          shape[MINO_WIDTH * y + x])
        gField[FIELD_WIDTH * (y + mino->y) + x + mino->x] = mino->type;
    }
  }
}

int mino_to_next_field(mino_t *mino) {
  int *shape = mino_to_shape(mino);
  for (int y = 0; y < MINO_HEIGHT; y++) {
    for (int x = 0; x < MINO_WIDTH; x++) {
      if ((0 <= y && y < NEXT_FIELD_HEIGHT) &&
          (0 <= x && x < NEXT_FIELD_WIDTH) && shape[MINO_WIDTH * y + x])
        gNextField[NEXT_FIELD_WIDTH * (y + 1) + x + 1] = mino->type;
    }
  }
}

int mino_to_scrn(mino_t *mino) {
  int *shape = mino_to_shape(mino);
  for (int y = 0; y < MINO_HEIGHT; y++) {
    for (int x = 0; x < MINO_WIDTH; x++) {
      if (shape[MINO_WIDTH * y + x])
        gScrn[SCRN_WIDTH * (y + mino->y + FIELD_OFFSET_Y) + x + mino->x +
              FIELD_OFFSET_X] = mino->type;
    }
  }
}

int move_mino(char cmd) {
  if (cmd == 'q') return 1;
  else if (cmd == 'a' || cmd == 'h') {
    gMovingMino->x--;
    if (is_collision(gMovingMino)) {
      gMovingMino->x++;
      return 0;
    }
  } else if (cmd == 'd' || cmd == 'l') {
    gMovingMino->x++;
    if (is_collision(gMovingMino)) {
      gMovingMino->x--;
      return 0;
    }
  } else if (cmd == 'r' || cmd == 'i') {
    gMovingMino->rot++;
    if (is_collision(gMovingMino)) {
      gMovingMino->rot--;
      return 0;
    }
  } else if (cmd == 's' || cmd == 'j') {
    gMovingMino->y++;
    if (is_collision(gMovingMino)) {
      gMovingMino->y--;

      mino_to_scrn(gMovingMino);

      for (int y = gMovingMino->y; y < gMovingMino->y + MINO_HEIGHT &&
                                   y < FIELD_OFFSET_Y + FIELD_HEIGHT - 1;
           y++) {
        if (is_completed_line(y))
          delite_line(y);
      }
      if (is_dead()) {
        printf("GAME_OVER\n");
        return 1;
      }
      create_new_mino();
      return 0;
    }
  } else if (cmd == 'S' || cmd == 'J') {
    while (1) {
      gMovingMino->y++;
      if (is_collision(gMovingMino)) {
        gMovingMino->y--;
        break;
      }
    }
    mino_to_scrn(gMovingMino);

    for (int y = gMovingMino->y; y < gMovingMino->y + MINO_HEIGHT &&
                                 y < FIELD_OFFSET_Y + FIELD_HEIGHT - 1;
         y++) {
      if (is_completed_line(y))
        delite_line(y);
    }
    if (is_dead()) {
      printf("GAME_OVER\n");
      return 1;
    }
    create_new_mino();
    return 0;
  } else return 0;

}

int is_collision(mino_t *mino) {
  int *shape = mino_to_shape(mino);
  for (int y = 0; y < MINO_HEIGHT; y++) {
    for (int x = 0; x < MINO_WIDTH; x++) {
      /*
      printf("s[%d][%d]: %d sc[%d][%d]: %d ", x, y, shape[MINO_WIDTH * y + x],
             x + mino->x + FIELD_OFFSET_X, y + mino->y + FIELD_OFFSET_Y,
             gScrn[SCRN_WIDTH * (y + mino->y + FIELD_OFFSET_Y) + x + mino->x +
                   FIELD_OFFSET_X]);
      printf("x: %d y: %d ",
             0 <= x + mino->x && x + mino->x < FIELD_WIDTH + FIELD_OFFSET_X,
             y + mino->y < FIELD_HEIGHT + FIELD_OFFSET_Y);
      */
      if (shape[MINO_WIDTH * y + x] != 0 &&
          SCRN_WIDTH * (y + mino->y + FIELD_OFFSET_Y) + x + mino->x +
                  FIELD_OFFSET_X <
              SCRN_SIZE &&
          gScrn[SCRN_WIDTH * (y + mino->y + FIELD_OFFSET_Y) + x + mino->x +
                FIELD_OFFSET_X] != 0) {
        return 1;
      }
    }
  }
  return 0;
}

int is_completed_line(int y) {
  for (int xi = 0; xi < FIELD_WIDTH; xi++) {
    /*
    printf("\nix:%d, iy:%d, sc:%d\n", xi, y + FIELD_OFFSET_Y,
           gScrn[SCRN_WIDTH * (y + FIELD_OFFSET_Y) + xi + FIELD_OFFSET_X] == 0);
    */
    if (gScrn[SCRN_WIDTH * (y + FIELD_OFFSET_Y) + xi + FIELD_OFFSET_X] == 0)
      return 0;
  }
  return 1;
}

void delite_line(int y) {
  for (int yi = y; yi > FIELD_OFFSET_Y; yi--) {
    for (int xi = 0; xi < FIELD_WIDTH; xi++) {
      gScrn[SCRN_WIDTH * (yi + FIELD_OFFSET_Y) + xi + FIELD_OFFSET_X] =
          gScrn[SCRN_WIDTH * (yi - 1 + FIELD_OFFSET_Y) + xi + FIELD_OFFSET_X];
    }
    // printf("dl %d\n", yi);
  }
  gScrn[SCRN_WIDTH * FIELD_OFFSET_Y + FIELD_OFFSET_X - 1] = 'b';
  gScrn[SCRN_WIDTH * FIELD_OFFSET_Y + FIELD_OFFSET_X + FIELD_WIDTH] = 'b';
}

int is_dead() {
  for (int xi = 0; xi < FIELD_WIDTH; xi++) {
    if (gScrn[SCRN_WIDTH * FIELD_OFFSET_Y + FIELD_OFFSET_X + xi] != 0)
      return 1;
  }
  return 0;
}

int main(void *args) {
  char cmd[10];
  init();
  create_new_mino();
  int t;

  while (1) {
    print_scrn();
    printf(">>");
    scanf("%s", cmd);
    if (move_mino(cmd[0]))  break;

    // 勝手に落ちないとつまらないので、5回に1回自動で落とす
    t++;
    if (t % 5 == 0) {
      if (move_mino('s')) break;
    }
  }
  return 0;
}
