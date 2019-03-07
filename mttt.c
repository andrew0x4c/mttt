/*
Copyright (c) Andrew Li 2019. This file is licensed under the GPLv3.
See https://github.com/andrew0x4c/mttt for more information,
including the full LICENSE file.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#define DEBUG if(0)

#ifndef N
#define N 3
#endif
#ifndef PREC
#define PREC 20
#endif

#define N2 (N*N)

#define LOOP(var) for(int64_t var = 0; var < N; var++)
#define DIAG(var) for(int64_t var = 0; var < 2; var++)

#define Player int8_t

#define FMT_MOVE "%"PRId64" %"PRId64" %"PRId64" %"PRId64

// Square

typedef struct Square {
    int8_t data[N][N];
    int8_t sumcol[N];
    int8_t sumrow[N];
    int8_t sumdia[2];
    int8_t win;
    uint8_t left;
} Square;

int8_t dia_data[2][N][N];

void init_dia_data() {
    LOOP(x) {
        dia_data[0][x][x] = 1;
        dia_data[1][x][N-1-x] = 1;
    }
}

const char* info_[] = {
    "-G", "-F", "-E", "-D", "-C", "-B", "-A", "-9",
    "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1",
    "<>",
    "+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8",
    "+9", "+A", "+B", "+C", "+D", "+E", "+F", "+G",
};
const char** info;

int8_t abs8(int8_t x) { return (x < 0) ? -x : x; }
//int8_t max8(int8_t x, int8_t y) { return (x >= y) ? x : y; }

void Square_init(Square* sq) {
    LOOP(x) LOOP(y) sq->data[x][y] = 0;
    LOOP(x) sq->sumcol[x] = 0;
    LOOP(y) sq->sumrow[y] = 0;
    DIAG(d) sq->sumdia[d] = 0;
    sq->win = 0;
    sq->left = N2;
}

void Square_play(Square* sq, int64_t x, int64_t y, Player p) {
    DEBUG assert(!sq->data[x][y]);
    sq->data[x][y] = p;
    sq->sumcol[x] += p;
    sq->sumrow[y] += p;
    DIAG(d) if(dia_data[d][x][y]) sq->sumdia[d] += p;
    long win = (abs8(sq->sumcol[x]) == N || abs8(sq->sumrow[y]) == N
             || abs8(sq->sumdia[0]) == N || abs8(sq->sumdia[1]) == N);
    sq->win = p & -win;
    sq->left--;
}

void Square_print(Square* sq) {
    LOOP(y) {
        printf("[");
        LOOP(x) printf(" %s", info[sq->data[x][y]]);
        printf(" ]\n");
    }
}

void Square_debug(Square* sq) {
    LOOP(y) {
        LOOP(x) printf("%s ", info[sq->data[x][y]]);
        printf("| %s\n", info[sq->sumrow[y]]);
    }
    LOOP(x) printf("-- "); printf("\n");
    LOOP(x) printf("%s ", info[sq->sumcol[x]]);
    printf("\n");
    printf("left=%d \\=%s /=%s win=%s\n", sq->left,
        info[sq->sumdia[0]], info[sq->sumdia[1]], info[sq->win]);
}

void Square_copy(Square* src, Square* dest) {
    LOOP(x) LOOP(y) dest->data[x][y] = src->data[x][y];
    LOOP(x) dest->sumcol[x] = src->sumcol[x];
    LOOP(y) dest->sumrow[y] = src->sumrow[y];
    DIAG(d) dest->sumdia[d] = src->sumdia[d];
    dest->win = src->win;
    dest->left = src->left;
}


// Meta

typedef struct Meta {
    Square small[N][N];
    Square big;
    int64_t lastx;
    int64_t lasty;
    int8_t any;
    uint16_t left;
} Meta;

void Meta_init(Meta* meta) {
    LOOP(x) LOOP(y) Square_init(&meta->small[x][y]);
    Square_init(&meta->big);
    meta->lastx = 0; meta->lasty = 0;
    meta->any = 1;
    meta->left = N2 * N2;
}

void Meta_play(Meta* meta,
        int64_t xx, int64_t yy, int64_t x, int64_t y, Player p) {
    DEBUG if(!meta->any) assert(xx == meta->lastx && yy == meta->lasty);
    Square* small = &meta->small[xx][yy];
    DEBUG assert(!small->data[x][y]);
    Square_play(small, x, y, p);
    if(small->win) {
        Square_play(&meta->big, xx, yy, p);
        meta->left -= small->left;
    }
    meta->lastx = x; meta->lasty = y;
    Square* next = &meta->small[x][y];
    meta->any = next->win || !next->left;
    meta->left--;
}

void Meta_print(Meta* meta) {
    LOOP(yy) {
        if(yy) {
            printf("[");
            LOOP(xx) {
                if(xx) printf(" *");
                LOOP(x) printf(" --");
            }
            printf(" ]\n");
        }
        LOOP(y) {
            printf("[");
            LOOP(xx) {
                if(xx) printf(" |");
                LOOP(x) printf(" %s", info[meta->small[xx][yy].data[x][y]]);
            }
            printf(" ]\n");
        }
    }
    printf("left=%d any=%d lastx=%"PRId64" lasty=%"PRId64" bigboard:\n",
        meta->left, meta->any, meta->lastx, meta->lasty);
    Square_print(&meta->big);
}

char* graphics[3][3] = {
    {"/---\\", "|   |", "\\---/"},
    {"|  - ", "| / |", " -  |"},
    {"\\   /", " >-< ", "/   \\"},
};

void Meta_print_2(Meta* meta) {
    LOOP(yy) {
        if(yy) {
            printf("[");
            LOOP(xx) {
                if(xx) printf(" *");
                LOOP(x) printf(" -");
            }
            printf(" ]\n");
        }
        LOOP(y) {
            printf("[");
            LOOP(xx) {
                int8_t ismeta = (!meta->any
                    && meta->lastx == xx && meta->lasty == yy);
                if(xx) printf("|");
                printf("%c", " <"[ismeta]);
                Square* small = &meta->small[xx][yy];
                if(small->win || !small->left) {
                    if(y < 3) {
                        printf("%s", graphics[meta->small[xx][yy].win+1][y]);
                        for(int64_t i = 0; i < N - 3; i++) printf("  ");
                    } else {
                        printf(" ");
                        for(int64_t i = 0; i < N - 1; i++) printf("  ");
                    }
                } else LOOP(x) {
                    if(x) printf(" ");
                    printf("%c", "O.X"[small->data[x][y]+1]);
                }
                printf("%c", " >"[ismeta]);
            }
            printf("]\n");
        }
    }
    /*
    | \   / | /---\ | |  -  |
    |  >-<  | |   | | | / | |
    | /   \ | \---/ |  -  | |
    */
}

void Meta_copy(Meta* src, Meta* dest) {
    LOOP(xx) LOOP(yy) Square_copy(&src->small[xx][yy], &dest->small[xx][yy]);
    Square_copy(&src->big, &dest->big);
    dest->lastx = src->lastx;
    dest->lasty = src->lasty;
    dest->any = src->any;
    dest->left = src->left;
}


// testing single square
// this code was only used for testing; however it could be useful
// if you ever want to get the AI to work for regular tic-tac-toe.

int64_t bestx, besty;
// will replacing with local make it slower?

int8_t best(Square* sq, Player p) { // -1=lose, 0=tie, 1=win
    if(sq->win || !sq->left) return sq->win * p;
    int8_t accum = -1;
    int64_t bx = 0, by = 0; // to stop the warning
    LOOP(x) LOOP(y) {
        if(sq->data[x][y]) continue;
        Square copy; Square_copy(sq, &copy);
        Square_play(&copy, x, y, p);
        int8_t curr = -best(&copy, -p);
        if(curr > accum) {
            accum = curr; bx = x; by = y;
        }
    }
    bestx = bx; besty = by;
    return accum;
}

int8_t get_rand_move(Square* sq, int64_t* xp, int64_t* yp) {
    if(!sq->left) return 0;
    int64_t c = rand() % sq->left; // I know
    int64_t i = 0;
    LOOP(x) LOOP(y) {
        if(sq->data[x][y]) continue;
        if(i++ == c) { *xp = x; *yp = y; }
    }
    return sq->left; // shouldn't happen
}

Player rand_moves(Square* sq, Player p) {
    Square copy; Square_copy(sq, &copy);
    int64_t x = 0, y = 0; // to stop the warning
    while(get_rand_move(&copy, &x, &y) && !copy.win) {
        Square_play(&copy, x, y, p);
        p = -p;
    }
    return copy.win;
}

int64_t eval(Square* sq, Player p) {
    int64_t nums[3] = {0, 0, 0};
    for(int64_t i = 0; i < 100000; i++) nums[rand_moves(sq, p)*p+1]++;
    return nums[2] - nums[0];
}

int64_t mcbest(Square* sq, Player p, int8_t depth) {
    // -1=lose, 0=tie, 1=win
    if(sq->win || !sq->left) return (sq->win * p) << (PREC + 1);
    if(depth == 0) return eval(sq, p);
    int64_t accum = -(1 << PREC) * 3;
    int64_t bx = 0, by = 0; // to stop the warning
    LOOP(x) LOOP(y) {
        if(sq->data[x][y]) continue;
        Square copy; Square_copy(sq, &copy);
        Square_play(&copy, x, y, p);
        int64_t curr = -mcbest(&copy, -p, depth - 1);
        if(curr > accum) {
            accum = curr; bx = x; by = y;
        }
    }
    bestx = bx; besty = by;
    return accum;
}


// actual mttt

int64_t mbestxx, mbestyy, mbestx, mbesty;
int64_t mctr;
// will replacing with local make it slower?

// manually loop when iterating to avoid rechecking same small
int8_t m_is_valid(Meta* meta, int64_t xx, int64_t yy, int64_t x, int64_t y) {
    Square* small = &meta->small[xx][yy];
    if(small->win || !small->left) return 0;
    if(small->data[x][y]) return 0;
    return 1;
}

int8_t m_is_valid_safe(Meta* meta,
        int64_t xx, int64_t yy, int64_t x, int64_t y) {
    // used for validating user input
    if(xx >= N || yy >= N || x >= N || y >= N) return 0;
    if(!(meta->any || (meta->lastx == xx && meta->lasty == yy))) return 0;
    return m_is_valid(meta, xx, yy, x, y);
}

int64_t m_num_moves(Meta* meta) {
    if(!meta->any) return meta->small[meta->lastx][meta->lasty].left;
    int64_t tot = 0;
    LOOP(xx) LOOP(yy) {
        Square* small = &meta->small[xx][yy];
        if(small->win || !small->left) continue;
        tot += small->left;
    }
    return tot;
}

int16_t m_get_rand_move(Meta* meta,
        int64_t* xxp, int64_t* yyp, int64_t* xp, int64_t* yp) {
    if(!meta->left) return 0;
    if(meta->any) {
        int64_t c = rand() % meta->left; // I know
        int64_t i = 0;
        LOOP(xx) LOOP(yy) {
            Square* small = &meta->small[xx][yy];
            if(small->win || !small->left) continue;
            LOOP(x) LOOP(y) {
                if(small->data[x][y]) continue;
                if(i++ == c) { *xxp = xx; *yyp = yy; *xp = x; *yp = y; }
            }
        }
        return meta->left;
    } else {
        Square* small = &meta->small[meta->lastx][meta->lasty];
        *xxp = meta->lastx;
        *yyp = meta->lasty;
        return get_rand_move(small, xp, yp);
    }
}

Player m_rand_moves(Meta* meta, Player p) {
    Meta copy; Meta_copy(meta, &copy);
    int64_t xx = 0, yy = 0, x = 0, y = 0; // to stop the warning
    while(m_get_rand_move(&copy, &xx, &yy, &x, &y) && !copy.big.win) {
        Meta_play(&copy, xx, yy, x, y, p);
        p = -p;
    }
    return copy.big.win;
}

int64_t m_eval(Meta* sq, Player p, int64_t nt) {
    int64_t nums[3] = {0, 0, 0};
    for(int64_t i = 0; i < nt; i++) nums[m_rand_moves(sq, p)+1]++;
    mctr += nt;
    return (((nums[2] - nums[0]) * p) << PREC) / nt;
}

// maybe refactor into an "iterator" for moves?

int64_t m_mcbest(Meta* meta, Player p, int64_t nt, int64_t minnt) {
    // -1=lose, 0=tie, 1=win
    if(meta->big.win || !meta->left) return (meta->big.win * p) << (PREC + 1);
    int64_t nmoves = m_num_moves(meta);
    if(nt / nmoves < minnt) {
        m_get_rand_move(meta, &mbestxx, &mbestyy, &mbestx, &mbesty);
        // in case this is the top level
        int64_t x = m_eval(meta, p, nt);
        return x;
    }
    int64_t accum = -(1 << PREC) * 3;
    int64_t bxx = 0, byy = 0, bx = 0, by = 0; // to stop the warning
    int64_t check = 0;
    LOOP(xx) LOOP(yy) {
        Square* small = &meta->small[xx][yy];
        if(small->win || !small->left) continue;
        if(!(meta->any || (meta->lastx == xx && meta->lasty == yy))) continue;
        LOOP(x) LOOP(y) {
            if(small->data[x][y]) continue;
            Meta copy; Meta_copy(meta, &copy);
            Meta_play(&copy, xx, yy, x, y, p);
            int64_t curr = -m_mcbest(&copy, -p, nt / nmoves, minnt);
            if(curr > accum) {
                accum = curr; bxx = xx; byy = yy; bx = x; by = y;
            }
            check++;
        }
    }
    mbestxx = bxx; mbestyy = byy; mbestx = bx; mbesty = by;
    return accum;
}

void m_player_move(Meta* meta, Player p) {
    char pc = "O.X"[p+1];
    while(1) {
        printf("(%c) ", pc);
        char cmd = getchar();
        if(cmd == '\n') {
            // nop
        } else if(cmd == 'p') {
            char c;
            while((c = getchar()) != '\n') {}
            Meta_print_2(meta);
        } else if(cmd == 'z') {
            int64_t data[4];
            int64_t i = 0;
            char c;
            while((c = getchar()) != '\n') {
                if(i < 4 && '0' <= c && c <= '9') data[i++] = c - '0';
            }
            if(i != 4) {
                printf("Need 4 coordinates (%"PRId64" given)\n", i);
                continue;
            }
            if(!m_is_valid_safe(meta,
                    data[0], data[1], data[2], data[3])) {
                printf("Invalid move " FMT_MOVE "\n",
                    data[0], data[1], data[2], data[3]);
                continue;
            }
            Meta_play(meta, data[0], data[1], data[2], data[3], p);
            break;
        } else if(cmd == 'm') {
            char c;
            while((c = getchar()) != '\n') {}
            LOOP(y) {
                printf("[");
                LOOP(x) printf(" %"PRId64"%"PRId64"", x, y);
                printf(" ]\n");
            }
        } else {
            char c;
            while((c = getchar()) != '\n') {}
            if(cmd != 'h') printf("Unrecognized command '%c'\n", cmd);
            printf("p: print; z: move; m: map; h: help\n");
        }
    }
}

void m_ai_move(Meta* meta, Player p, int64_t nt, int64_t minnt) {
    char pc = "O.X"[p+1];
    mbestxx = 42; mbestyy = 42; mbestx = 42; mbesty = 42;
    mctr = 0;
    printf("(%c) thinking...\n", pc);
    int64_t result = m_mcbest(meta, p, nt, minnt);
    printf("(%c) opinion %"PRId64"%%\n", pc, (result * 100) >> PREC);
    printf("(%c) move " FMT_MOVE "\n", pc, mbestxx, mbestyy, mbestx, mbesty);
    printf("(%c) in %"PRId64" evals\n", pc, mctr);
    Meta_play(meta, mbestxx, mbestyy, mbestx, mbesty, p);
}

char* usage_str = (
    "Meta tic-tac-toe (N = %d)\n"
    "usage: %s [--help] [--sizeof] [--seed (none)] [--start 1]\n"
    "        [--p1ai 1] [--p1nt 100000] [--p1minnt 40]\n"
    "        [--p2ai 1] [--p2nt 100000] [--p2minnt 40]\n"
    "  --help        display help and exit\n"
    "  --sizeof      display size of structs and exit\n"
    "  --seed        set random seed\n"
    "  --start       select starting player (1 = X, 2 = O)\n"
    "  --p(1|2)ai    enable AI for given player (0 = off, 1 = on)\n"
    "  --p(1|2)nt    maximum number of playouts for a move\n"
    "  --p(1|2)minnt minimum number of playouts for a leaf node\n"
);

#define usage() fprintf(stderr, usage_str, N, argv[0])

#define error(...) do { fprintf(stderr, __VA_ARGS__); usage(); exit(1); \
    } while(0)

int64_t is_int(char* str) {
    while(*str) {
        if(!('0' <= *str && *str <= '9')) return 0;
        str++;
    }
    return 1;
}

#define INTARG(s) if(strcmp(*curr, "--" #s) == 0) {                 \
        curr++;                                                     \
        if(!*curr) {                                                \
            error("Missing argument for --" #s "\n");               \
        }                                                           \
        if(!is_int(*curr)) {                                        \
            error("Invalid argument for --" #s ": %s\n", *curr);    \
        }                                                           \
        s = atol(*curr);                                            \
    }

int main(int argc, char** argv) {
    info = info_ + 16;
    init_dia_data();

    int64_t p1ai = 1;
    int64_t p1nt = 100000;
    int64_t p1minnt = 40;
    int64_t p2ai = 1;
    int64_t p2nt = 100000;
    int64_t p2minnt = 40;
    int64_t seed = time(NULL);
    int64_t start = 1;

    char** curr = argv + 1;
    while(*curr) {
        if(strcmp(*curr, "--help") == 0) {
            usage(); exit(1);
        }
        else if(strcmp(*curr, "--sizeof") == 0) {
            fprintf(stderr, "sizeof(Square) == %lu\n", sizeof(Square));
            fprintf(stderr, "sizeof(Meta) == %lu\n", sizeof(Meta));
            exit(1);
        }
        else INTARG(seed) else INTARG(start)
        else INTARG(p1ai) else INTARG(p1nt) else INTARG(p1minnt)
        else INTARG(p2ai) else INTARG(p2nt) else INTARG(p2minnt)
        else error("Unrecognized argument %s\n", *curr);
        curr++;
    }
    if(!(p1ai == 0 || p1ai == 1)) {
        error("Invalid value of p1ai: %"PRId64"\n", p1ai);
    }
    if(!(p2ai == 0 || p2ai == 1)) {
        error("Invalid value of p2ai: %"PRId64"\n", p2ai);
    }
    if(!(start == 1 || start == 2)) {
        error("Invalid starting player: %"PRId64"\n", start);
    }
    srand(seed);

    Meta meta; Meta_init(&meta);
    Player p = 3 - 2 * start; // 1 -> 1, 2 -> -1
    while(1) {
        Meta_print_2(&meta);
        if(p > 0) {
            if(p1ai) m_ai_move(&meta, p, p1nt, p1minnt);
            else m_player_move(&meta, p);
        } else {
            if(p2ai) m_ai_move(&meta, p, p2nt, p2minnt);
            else m_player_move(&meta, p);
        }
        p = -p;
        if(meta.big.win || !m_num_moves(&meta)) break;
    }
    Meta_print_2(&meta);
    printf("WINNER %c\n", "OTX"[meta.big.win+1]);
}
// TODO: load/save

