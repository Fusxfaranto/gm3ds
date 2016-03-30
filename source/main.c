#include <string.h>
#include <stdio.h>
#include <3ds.h>

#include "font.h"

#define DEBUG(x) x

#define ASSERT(b, ...) DEBUG(                                   \
        if (!(b))                                               \
        {                                                       \
            printf("assertion fired at line %d:\n", __LINE__);  \
            printf(__VA_ARGS__);                                \
            for (;;) {}                                         \
        })


typedef struct Game Game;


typedef enum
{
    KN__SONIC_DROP = BIT(0),
    KN__HARD_DROP = BIT(1),
    KN__SOFT_DROP = BIT(2),
    KN__LEFT = BIT(3),
    KN__RIGHT = BIT(4),
    KN__A = BIT(5),
    KN__B = BIT(6),
    KN__C = BIT(7),
} KeyName;
#define NUM_KEY_NAMES 8

#define KEY_NONE 0


typedef struct
{
    u8 b;
    u8 g;
    u8 r;
} Color;

const Color TETROMINO_COLORS[7] = {
    {  0,   0, 255},
    {255, 255,   0},
    {  0, 127, 255},
    {255,   0,   0},
    {255,   0, 255},
    {  0, 255,   0},
    {  0, 255, 255}};

#define PIECE__I 0
#define PIECE__T 1
#define PIECE__L 2
#define PIECE__J 3
#define PIECE__S 4
#define PIECE__Z 5
#define PIECE__O 6

const s8 TETROMINOS[7][4][4][4] =
{{{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},  // I
  {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},
  {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}}},
 {{{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},  // T
  {{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
  {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
 {{{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},  // L
  {{1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
  {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}},
 {{{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},  // J
  {{0, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
  {{0, 1, 1, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
 {{{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},  // S
  {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
  {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
 {{{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},  // Z
  {{0, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
  {{0, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
 {{{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},  // O
  {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
  {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}}};

typedef struct
{
    int x;
    int y;
} Position;

// includes hidden rows
#define BOARD_HEIGHT 22
#define BOARD_WIDTH 10

#define HISTORY_LENGTH 4
#define HISTORY_TRIES 4

#define MAX_SECTIONS_LENGTH 10
const u32 SECTION_SCORE_REQ[MAX_SECTIONS_LENGTH] = {0, 0, 12000, 0, 40000, 0, 0, 0, 0, 126000};
const u32 SECTION_TIME_REQ[MAX_SECTIONS_LENGTH] = {-1, -1, 15300, -1, 27000, 0, 0, 0, 0, 48600};

#define NUM_GRADES 19
const char GRADE_STRINGS[NUM_GRADES][2] = {" 9", " 8", " 7", " 6", " 5", " 4", " 3", " 2", " 1",
                                           "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9", "GM"};
#define GRADE_SCORES__HEXMARK ((u32)-1)
#define GRADE_SCORES__TRIDASH ((u32)-2)
const u32 GRADE_SCORES[NUM_GRADES + 1] =
{
    0, 400, 800, 1400, 2000, 3500, 5500, 8000, 12000,
    16000, 22000, 30000, 40000, 52000, 66000, 82000, 100000, 120000,
    GRADE_SCORES__HEXMARK, GRADE_SCORES__TRIDASH
};

typedef struct
{
    int tet;
    Position pos;
    int rot;
} Piece;

typedef enum
{
    PS__IN_GAME,
    PS__PAUSED,
    PS__LOSE_SCREEN,
    PS__MAIN_MENU,
    PS__QUITTING,
} ProgState;

#define NUM_MENU_ITEMS 3
#define MAX_MENU_ITEM_LENGTH 40

typedef struct
{
    u8 cursor_pos;
    char item_strs[NUM_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];
    void (*item_callbacks[NUM_MENU_ITEMS])(Game*, u32, u8);
} Menu;

typedef struct
{
    int level;
    u16 gravity;
    u8 are;
    u8 line_are;
    u8 das;
    u8 lock_delay;
} Speed;

#define MAX_SPEED_CHANGES 50

typedef struct
{
    int sections[MAX_SECTIONS_LENGTH];
    Speed speeds[MAX_SPEED_CHANGES];
    u8 sections_length;
    u8 speeds_length;
} GameParams;

#define NUM_MODES 4
#define DEFAULT__TGM 0
#define DEFAULT__TGM20G 1
#define DEFAULT__DEATH 2
#define DEFAULT__TGM20GILD 3
const char *MODE_NAMES[NUM_MODES] = {"TGM", "TGM 20G", "T.A. DEATH", "TGM 20G (INF LD)"};

#define INF_LOCK_DELAY 255

struct Game
{
    Menu menu;

    ProgState prog_state;
    u64 rngstate;
    s32 timer;

    u32 curr_keys;
    u32 old_keys;
    PAD_KEY key_config[NUM_KEY_NAMES];

    s8 board[BOARD_HEIGHT][BOARD_WIDTH];
    int history[HISTORY_LENGTH];
    Piece piece;
    Piece next_piece;
    bool piece_inactive;

    u32 score;
    u8 combo;
    u8 soft_drop_frames;
    bool gm_eligible;

    GameParams defaults[NUM_MODES];
    GameParams p;
    u8 mode_index;

    int level;
    u8 current_section_index;
    u8 current_speed_index;

    u16 gcount;
    u8 ldcount;
    u8 arecount;
    u8 dascount;
    s8 das_direction;
};


static u8 current_grade(Game *g)
{
    if (g->level >= 999 && g->gm_eligible)
    {
        return NUM_GRADES;
    }
    int i;
    for (i = NUM_GRADES - 1; GRADE_SCORES[i] > g->score; i--) {ASSERT(i != (u32)-1, "current_grade out of bounds");}
    return i;
}

static u8 check_clear_lines(s8 board[BOARD_HEIGHT][BOARD_WIDTH], u8 *bravo)
{
    u8 line_count = 0;

    *bravo = 4;

    for (int row = 2; row < BOARD_HEIGHT; row++)
    {
        bool line = true;
        for (int col = 0; col < BOARD_WIDTH; col++)
        {
            if (board[row][col] == 0)
            {
                line = false;
                //break; //TODO: is this ok
            }
            else
            {
                *bravo = 1;
            }
        }

        if (line)
        {
            line_count++;
            for (int col = 0; col < BOARD_WIDTH; col++)
            {
                board[row][col] = 0;
            }
        }
    }
    return line_count;
}

static void cascade(s8 board[BOARD_HEIGHT][BOARD_WIDTH])
{
    // TODO: should this start at 2
    for (int row = 2; row < BOARD_HEIGHT; row++)
    {
        bool empty = true;
        for (int col = 0; col < BOARD_WIDTH; col++)
        {
            if (board[row][col] != 0) {
                empty = false;
                //break; //TODO: is this ok
            }
        }

        if (empty)
        {
            //TODO maybe make a little nicer once everything confirmed working
            for (int row2 = row - 1; row2 >= 0; row2--)
            {
                for (int col = 0; col < BOARD_WIDTH; col++)
                {
                    board[row2 + 1][col] = board[row2][col];
                }
            }
            for (int col = 0; col < BOARD_WIDTH; col++)
            {
                board[0][col] = 0;
            }
        }
    }
}

static bool check_move(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (TETROMINOS[piece->tet][piece->rot][row][col] != 0)
            {
                int brow = row + piece->pos.y;
                int bcol = col + piece->pos.x;
                if (brow < 0 || brow >= BOARD_HEIGHT || bcol < 0 || bcol >= BOARD_WIDTH
                    || board[brow][bcol] != 0)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

static inline bool is_on_floor(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece)
{
    piece->pos.y += 1;
    bool result = !check_move(board, piece);
    piece->pos.y -= 1;
    return result;
}

static inline bool kick_not_blocked(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece)
{
    if (piece->tet == PIECE__I)
    {
        return false;
    }

    // logic courtesy of raincomplex :)
    if (piece->tet == PIECE__L || piece->tet == PIECE__J || piece->tet == PIECE__T)
    {
        if (board[piece->pos.y][piece->pos.x + 1] || board[piece->pos.y + 1][piece->pos.x + 1]
            || board[piece->pos.y + 2][piece->pos.x + 1])
        {
            if ((piece->tet == PIECE__L && board[piece->pos.y][piece->pos.x])
                || (piece->tet == PIECE__J && board[piece->pos.y][piece->pos.x + 2]))
            {
                return true;
            }
            return false;
        }
    }
    return true;
}

static inline void rotate(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece, int amt, bool can_kick)
{
    ASSERT(amt > 0 && amt < 4, "arg to rotate (%d) out of bounds", amt);
    piece->rot += amt;
    if (piece->rot > 3)
    {
        piece->rot -= 4;
    }
    if (!check_move(board, piece))
    {
        if (can_kick && kick_not_blocked(board, piece))
        {
            piece->pos.x += 1;
            if (!check_move(board, piece))
            {
                piece->pos.x -= 2;
                if (!check_move(board, piece))
                {
                    piece->pos.x += 1;
                }
                else
                {
                    // successfully kicked left
                    return;
                }
            }
            else
            {
                // successfully kicked right
                return;
            }
        }

        // failed to rotate, undo rotation
        if (piece->rot < amt)
        {
            piece->rot += 4 - amt;
        }
        else
        {
            piece->rot -= amt;
        }
    }
}

static inline void horiz_move(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece, s8 amt)
{
    piece->pos.x += amt;
    if (!check_move(board, piece))
    {
        piece->pos.x -= amt;
    }
}

static inline bool next_piece__rule30(bool a, bool b, bool c)
{
    if (a && (b || c)) { return false; }
    else if (!a && !b && !c) { return false; }
    else { return true; }
}

static bool next_piece__rng(u64* state)
{
    u64 new_state = next_piece__rule30(*state & (1ULL << 63ULL),
                                       *state & (1ULL << 0ULL),
                                       *state & (1ULL << 1ULL));
    for (u64 i = 2ULL; i < (1ULL << 63ULL); i <<= 1ULL)
    {
        new_state += next_piece__rule30(*state & (i >> 1ULL),
                                        *state & i,
                                        *state & (i << 1ULL)) * i;
    }
/*    new_state += next_piece__rule30(*state & (1ULL << 62ULL),
 *state & (1ULL << 63ULL),
 *state & (1ULL << 0ULL)) << 63ULL;*/
    new_state += next_piece__rule30(*state & (1ULL << 62ULL),
                                    *state & (1ULL << 63ULL),
                                    *state & (1ULL << 0ULL)) * (1ULL << 63ULL);
    *state = new_state;

    return new_state & (1ULL << 32ULL);
}

static int next_piece__randpiece(u64* state)
{
    int piece = next_piece__rng(state);
    piece += next_piece__rng(state) << 1;
    piece += next_piece__rng(state) << 2;
    ASSERT(piece <= 7 && piece >= 0, "piece out of bounds: %d", piece);
    if (piece == 7) { return next_piece__randpiece(state); }
    else { return piece; }
}

static void next_piece(Piece *piece, Piece *new_piece, u64 *state,
                       int history[HISTORY_LENGTH], bool first_piece)
{
    *piece = *new_piece;
    piece->pos.x = 3;
    piece->pos.y = 1;
    piece->rot = 0;

    bool found;
    for (int i = 0; i < HISTORY_TRIES; i++)
    {
        found = false;
        new_piece->tet = next_piece__randpiece(state);
        //DEBUG(printf("generated piece %d, ", new_piece->tet));
        for (int j = 0; j < HISTORY_LENGTH; j++)
        {
            if (history[j] == new_piece->tet)
            {
                //DEBUG(printf("found in history\n"));
                found = true;
                break;
            }
        }
        if (!found)
        {
            //DEBUG(printf("done generating\n"));
            break;
        }
    }
    /* DEBUG( */
    /*     if (found) */
    /*     { */
    /*         printf("gave up, generated history piece\n"); */
    /*     } */
    /*     ); */

    if (first_piece)
    {
        while (new_piece->tet == PIECE__S || new_piece->tet == PIECE__Z || new_piece->tet == PIECE__O)
        {
            new_piece->tet = next_piece__randpiece(state);
        }
    }

    for (int i = HISTORY_LENGTH - 1; i > 0; i--)
    {
        history[i] = history[i - 1];
    }
    history[0] = new_piece->tet;

    ASSERT(new_piece->tet < 7, "piece out of bounds (after history check): %d", new_piece->tet);
}

static inline void lock(Game* g)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (TETROMINOS[g->piece.tet][g->piece.rot][row][col] != 0)
            {
                ASSERT(g->board[row + g->piece.pos.y][col + g->piece.pos.x] == 0, "%d %d    %d",
                       row + g->piece.pos.y, col + g->piece.pos.x, g->piece.tet);
                g->board[row + g->piece.pos.y][col + g->piece.pos.x] = g->piece.tet + 1;
            }
        }
    }

    u8 bravo;
    u8 lines_cleared = check_clear_lines(g->board, &bravo);
    if (lines_cleared == 0)
    {
        g->arecount = g->p.speeds[g->current_speed_index].are;
        g->combo = 1;
    }
    else
    {
        g->arecount = g->p.speeds[g->current_speed_index].line_are;
        ASSERT(lines_cleared > 0 && lines_cleared < 5, "%hhd", lines_cleared);

        // https://tetris.wiki/Tetris_The_Grand_Master#Scoring
        // note that combo should be updated after calculating score, not before
        // the page (as of 3/22/16) is wrong about this
        g->score += ((g->level + lines_cleared - 1) / 4 + 1 + g->soft_drop_frames)
            * lines_cleared * (2 * lines_cleared - 1) * g->combo * bravo;
        DEBUG(printf("level: %d, lines: %hhu, sdf: %hhu, combo: %hhu, bravo: %hhu\n",
                     g->level, lines_cleared, g->soft_drop_frames, g->combo, bravo));
        g->combo += 2 * lines_cleared - 2;

        g->level += lines_cleared;
        if (g->level >= g->p.sections[g->current_section_index]
            && g->current_section_index < g->p.sections_length - 1)
        {
            if (g->score < SECTION_SCORE_REQ[g->current_section_index] ||
                g->timer >= SECTION_TIME_REQ[g->current_section_index])
            {
                g->gm_eligible = false;
            }

            g->current_section_index++;
        }
    }
    g->piece_inactive = true;


}


static inline void game_reset(Game* g)
{
    //g->rngstate = 1311666606263354484;
    g->rngstate = osGetTime();
    DEBUG(printf("%llu\n", g->rngstate));
    for (int i = 0; i < 100; i++) next_piece__randpiece(&(g->rngstate));
    DEBUG(printf("%llu\n", g->rngstate));

    g->timer = -60; // to reach zero when piece actually starts

    g->curr_keys = 0;
    g->old_keys = 0;

    memset(g->board, 0, BOARD_HEIGHT * BOARD_WIDTH);

    _Static_assert(HISTORY_LENGTH == 4, "need to update history initialization");
    g->history[0] = PIECE__Z;
    g->history[1] = PIECE__Z;
    g->history[2] = PIECE__Z;
    g->history[3] = PIECE__Z;

    next_piece(&(g->piece), &(g->next_piece), &(g->rngstate), g->history, true);

    g->level = 0;
    g->current_section_index = 0;
    g->current_speed_index = 0;

    g->score = 0;
    g->combo = 1;
    g->soft_drop_frames = 0;
    g->gm_eligible = true;

    g->piece_inactive = true;
    g->gcount = 0;
    g->ldcount = g->p.speeds[g->current_speed_index].lock_delay;
    g->arecount = -g->timer;
    g->dascount = 0;
    g->das_direction = 0;
}


static void menu_item_callback_start(Game* g, u32 k_down, u8 menu_idx)
{
    if (k_down & KEY_A)
    {
        game_reset(g);
        g->prog_state = PS__IN_GAME;
    }
}

static void menu_item_callback_mode_change(Game* g, u32 k_down, u8 menu_idx)
{
    if (k_down & KEY_DLEFT)
    {
        g->mode_index += NUM_MODES;
        g->mode_index--;
    }
    if (k_down & KEY_RIGHT)
    {
        g->mode_index++;
    }
    g->mode_index %= NUM_MODES;

    memcpy(&g->p, &g->defaults[g->mode_index], sizeof(GameParams));

    strcpy(g->menu.item_strs[menu_idx] + 8, MODE_NAMES[g->mode_index]);
    strcpy(g->menu.item_strs[menu_idx] + 8 + strlen(MODE_NAMES[g->mode_index]), " >");

    if (k_down & KEY_A)
    {
        menu_item_callback_start(g, k_down, menu_idx);
    }
}

static void menu_item_callback_quit(Game* g, u32 k_down, u8 menu_idx)
{
    if (k_down & KEY_A)
    {
        g->prog_state = PS__QUITTING;
    }
}

//static void menu_item_callback_noop(Game* g, u32 k_down, u8 menu_idx) {}

static inline void game_init(Game* g)
{
    g->prog_state = PS__MAIN_MENU;

    _Static_assert(NUM_KEY_NAMES == 8, "need to update key config");
    g->key_config[0 /*KN__SONIC_DROP*/] = KEY_DUP;
    g->key_config[1 /*KN__HARD_DROP*/] = KEY_NONE;
    g->key_config[2 /*KN__SOFT_DROP*/] = KEY_DDOWN;
    g->key_config[3 /*KN__LEFT*/] = KEY_DLEFT;
    g->key_config[4 /*KN__RIGHT*/] = KEY_DRIGHT;
    g->key_config[5 /*KN__A*/] = KEY_B;
    g->key_config[6 /*KN__B*/] = KEY_A;
    g->key_config[7 /*KN__C*/] = KEY_NONE;

    g->defaults[DEFAULT__TGM].sections_length = 10;
    for (int i = 0; i < 9; i++) g->defaults[DEFAULT__TGM].sections[i] = (i + 1) * 100;
    g->defaults[DEFAULT__TGM].sections[9] = 999;

    g->defaults[DEFAULT__TGM].speeds_length = 30;
    for (int i = 0; i < g->defaults[DEFAULT__TGM].speeds_length; i++)
    {
        g->defaults[DEFAULT__TGM].speeds[i] = (Speed){0, 0, 30, 71, 14, 30};
    }
    g->defaults[DEFAULT__TGM].speeds[0].level = 0;
    g->defaults[DEFAULT__TGM].speeds[0].gravity = 4;
    g->defaults[DEFAULT__TGM].speeds[1].level = 30;
    g->defaults[DEFAULT__TGM].speeds[1].gravity = 6;
    g->defaults[DEFAULT__TGM].speeds[2].level = 35;
    g->defaults[DEFAULT__TGM].speeds[2].gravity = 8;
    g->defaults[DEFAULT__TGM].speeds[3].level = 40;
    g->defaults[DEFAULT__TGM].speeds[3].gravity = 10;
    g->defaults[DEFAULT__TGM].speeds[4].level = 50;
    g->defaults[DEFAULT__TGM].speeds[4].gravity = 12;
    g->defaults[DEFAULT__TGM].speeds[5].level = 60;
    g->defaults[DEFAULT__TGM].speeds[5].gravity = 16;
    g->defaults[DEFAULT__TGM].speeds[6].level = 70;
    g->defaults[DEFAULT__TGM].speeds[6].gravity = 32;
    g->defaults[DEFAULT__TGM].speeds[7].level = 80;
    g->defaults[DEFAULT__TGM].speeds[7].gravity = 48;
    g->defaults[DEFAULT__TGM].speeds[8].level = 90;
    g->defaults[DEFAULT__TGM].speeds[8].gravity = 64;
    g->defaults[DEFAULT__TGM].speeds[9].level = 100;
    g->defaults[DEFAULT__TGM].speeds[9].gravity = 80;
    g->defaults[DEFAULT__TGM].speeds[10].level = 120;
    g->defaults[DEFAULT__TGM].speeds[10].gravity = 96;
    g->defaults[DEFAULT__TGM].speeds[11].level = 140;
    g->defaults[DEFAULT__TGM].speeds[11].gravity = 112;
    g->defaults[DEFAULT__TGM].speeds[12].level = 160;
    g->defaults[DEFAULT__TGM].speeds[12].gravity = 128;
    g->defaults[DEFAULT__TGM].speeds[13].level = 170;
    g->defaults[DEFAULT__TGM].speeds[13].gravity = 144;
    g->defaults[DEFAULT__TGM].speeds[14].level = 200;
    g->defaults[DEFAULT__TGM].speeds[14].gravity = 4;
    g->defaults[DEFAULT__TGM].speeds[15].level = 220;
    g->defaults[DEFAULT__TGM].speeds[15].gravity = 32;
    g->defaults[DEFAULT__TGM].speeds[16].level = 230;
    g->defaults[DEFAULT__TGM].speeds[16].gravity = 64;
    g->defaults[DEFAULT__TGM].speeds[17].level = 233;
    g->defaults[DEFAULT__TGM].speeds[17].gravity = 96;
    g->defaults[DEFAULT__TGM].speeds[18].level = 236;
    g->defaults[DEFAULT__TGM].speeds[18].gravity = 128;
    g->defaults[DEFAULT__TGM].speeds[19].level = 239;
    g->defaults[DEFAULT__TGM].speeds[19].gravity = 160;
    g->defaults[DEFAULT__TGM].speeds[20].level = 243;
    g->defaults[DEFAULT__TGM].speeds[20].gravity = 192;
    g->defaults[DEFAULT__TGM].speeds[21].level = 247;
    g->defaults[DEFAULT__TGM].speeds[21].gravity = 224;
    g->defaults[DEFAULT__TGM].speeds[22].level = 251;
    g->defaults[DEFAULT__TGM].speeds[22].gravity = 256;
    g->defaults[DEFAULT__TGM].speeds[23].level = 300;
    g->defaults[DEFAULT__TGM].speeds[23].gravity = 512;
    g->defaults[DEFAULT__TGM].speeds[24].level = 330;
    g->defaults[DEFAULT__TGM].speeds[24].gravity = 768;
    g->defaults[DEFAULT__TGM].speeds[25].level = 360;
    g->defaults[DEFAULT__TGM].speeds[25].gravity = 1024;
    g->defaults[DEFAULT__TGM].speeds[26].level = 400;
    g->defaults[DEFAULT__TGM].speeds[26].gravity = 1280;
    g->defaults[DEFAULT__TGM].speeds[27].level = 420;
    g->defaults[DEFAULT__TGM].speeds[27].gravity = 1024;
    g->defaults[DEFAULT__TGM].speeds[28].level = 450;
    g->defaults[DEFAULT__TGM].speeds[28].gravity = 768;
    g->defaults[DEFAULT__TGM].speeds[29].level = 500;
    g->defaults[DEFAULT__TGM].speeds[29].gravity = 5120;

    memcpy(&g->defaults[DEFAULT__TGM20G], &g->defaults[DEFAULT__TGM], sizeof(GameParams));
    for (int i = 0; i < g->defaults[DEFAULT__TGM20G].speeds_length; i++)
    {
        g->defaults[DEFAULT__TGM20G].speeds[i].gravity = 5120;
    }

    memcpy(&g->defaults[DEFAULT__DEATH], &g->defaults[DEFAULT__TGM20G], sizeof(GameParams));
    g->defaults[DEFAULT__DEATH].speeds_length = 9;
    g->defaults[DEFAULT__DEATH].speeds[0].level = 0;
    g->defaults[DEFAULT__DEATH].speeds[0].are = 16;
    g->defaults[DEFAULT__DEATH].speeds[0].line_are = 24;
    g->defaults[DEFAULT__DEATH].speeds[0].das = 10;
    g->defaults[DEFAULT__DEATH].speeds[0].lock_delay = 30;
    g->defaults[DEFAULT__DEATH].speeds[1].level = 101;
    g->defaults[DEFAULT__DEATH].speeds[1].are = 12;
    g->defaults[DEFAULT__DEATH].speeds[1].line_are = 12;
    g->defaults[DEFAULT__DEATH].speeds[1].das = 10;
    g->defaults[DEFAULT__DEATH].speeds[1].lock_delay = 26;
    g->defaults[DEFAULT__DEATH].speeds[2].level = 200;
    g->defaults[DEFAULT__DEATH].speeds[2].are = 12;
    g->defaults[DEFAULT__DEATH].speeds[2].line_are = 12;
    g->defaults[DEFAULT__DEATH].speeds[2].das = 9;
    g->defaults[DEFAULT__DEATH].speeds[2].lock_delay = 26;
    g->defaults[DEFAULT__DEATH].speeds[3].level = 201;
    g->defaults[DEFAULT__DEATH].speeds[3].are = 12;
    g->defaults[DEFAULT__DEATH].speeds[3].line_are = 12;
    g->defaults[DEFAULT__DEATH].speeds[3].das = 9;
    g->defaults[DEFAULT__DEATH].speeds[3].lock_delay = 22;
    g->defaults[DEFAULT__DEATH].speeds[4].level = 300;
    g->defaults[DEFAULT__DEATH].speeds[4].are = 12;
    g->defaults[DEFAULT__DEATH].speeds[4].line_are = 12;
    g->defaults[DEFAULT__DEATH].speeds[4].das = 8;
    g->defaults[DEFAULT__DEATH].speeds[4].lock_delay = 22;
    g->defaults[DEFAULT__DEATH].speeds[5].level = 301;
    g->defaults[DEFAULT__DEATH].speeds[5].are = 6;
    g->defaults[DEFAULT__DEATH].speeds[5].line_are = 12;
    g->defaults[DEFAULT__DEATH].speeds[5].das = 8;
    g->defaults[DEFAULT__DEATH].speeds[5].lock_delay = 18;
    g->defaults[DEFAULT__DEATH].speeds[6].level = 400;
    g->defaults[DEFAULT__DEATH].speeds[6].are = 6;
    g->defaults[DEFAULT__DEATH].speeds[6].line_are = 12;
    g->defaults[DEFAULT__DEATH].speeds[6].das = 6;
    g->defaults[DEFAULT__DEATH].speeds[6].lock_delay = 18;
    g->defaults[DEFAULT__DEATH].speeds[7].level = 401;
    g->defaults[DEFAULT__DEATH].speeds[7].are = 5;
    g->defaults[DEFAULT__DEATH].speeds[7].line_are = 10;
    g->defaults[DEFAULT__DEATH].speeds[7].das = 6;
    g->defaults[DEFAULT__DEATH].speeds[7].lock_delay = 15;
    g->defaults[DEFAULT__DEATH].speeds[8].level = 500;
    g->defaults[DEFAULT__DEATH].speeds[8].are = 4;
    g->defaults[DEFAULT__DEATH].speeds[8].line_are = 8;
    g->defaults[DEFAULT__DEATH].speeds[8].das = 6;
    g->defaults[DEFAULT__DEATH].speeds[8].lock_delay = 15;

    memcpy(&g->defaults[DEFAULT__TGM20GILD], &g->defaults[DEFAULT__TGM20G], sizeof(GameParams));
    for (int i = 0; i < g->defaults[DEFAULT__TGM20GILD].speeds_length; i++)
    {
        g->defaults[DEFAULT__TGM20GILD].speeds[i].lock_delay = INF_LOCK_DELAY;
    }

    memcpy(&g->p, &g->defaults[DEFAULT__TGM], sizeof(GameParams));

    g->menu.cursor_pos = 0;
    strcpy(g->menu.item_strs[0], "START");
    g->menu.item_callbacks[0] = menu_item_callback_start;
    strcpy(g->menu.item_strs[1], "MODE: < PLACEHOLDER >");
    g->menu.item_callbacks[1] = menu_item_callback_mode_change;
    menu_item_callback_mode_change(g, 0, 1);
    strcpy(g->menu.item_strs[2], "QUIT");
    g->menu.item_callbacks[2] = menu_item_callback_quit;

    game_reset(g);
}


static inline void game_process_input(Game* g)
{
    u32 k_held = hidKeysHeld();

    g->old_keys = g->curr_keys;

    for (int i = 0; i < NUM_KEY_NAMES; i++)
    {
        if (k_held & g->key_config[i])
        {
            g->curr_keys |= 1 << i;
        }
        else
        {
            g->curr_keys &= ~(1 << i);
        }
    }
}

static inline void game_update(Game* g)
{
    g->timer++;

    if (g->curr_keys & KN__LEFT)
    {
        if (g->das_direction == -1)
        {
            if (g->dascount < g->p.speeds[g->current_speed_index].das)
            {
                g->dascount += 1;
            }
        }
        else
        {
            g->das_direction = -1;
            g->dascount = 1;
        }
    }
    else if (g->curr_keys & KN__RIGHT)
    {
        if (g->das_direction == 1)
        {
            if (g->dascount < g->p.speeds[g->current_speed_index].das)
            {
                g->dascount += 1;
            }
        }
        else
        {
            g->das_direction = 1;
            g->dascount = 1;
        }
    }
    else
    {
        g->dascount = 0;
    }

    if (g->arecount > 0)
    {
        g->arecount -= 1;
        if (g->arecount == g->p.speeds[g->current_speed_index].line_are / 2)
        {
            cascade(g->board);
        }
    }
    if (g->arecount == 0)
    {
        if (!g->piece_inactive)
        {
            // rotation
            if ((g->curr_keys & KN__A) && !(g->old_keys & KN__A))
            {
                rotate(g->board, &(g->piece), 3, true);
            }
            else if ((g->curr_keys & KN__B) && !(g->old_keys & KN__B))
            {
                rotate(g->board, &(g->piece), 1, true);
            }
            else if ((g->curr_keys & KN__C) && !(g->old_keys & KN__C))
            {
                rotate(g->board, &(g->piece), 3, true);
            }

            // movement
            if (g->dascount >= g->p.speeds[g->current_speed_index].das)
            {
                ASSERT(g->p.speeds[g->current_speed_index].das == g->dascount, " ");
                horiz_move(g->board, &(g->piece), g->das_direction);
            }
            else if (((g->curr_keys & KN__LEFT) && !(g->old_keys & KN__LEFT)))
            {
                horiz_move(g->board, &(g->piece), -1);
            }
            else if ((g->curr_keys & KN__RIGHT) && !(g->old_keys & KN__RIGHT))
            {
                horiz_move(g->board, &(g->piece), 1);
            }

            // soft drop score bonus
            if (g->curr_keys & KN__SOFT_DROP)
            {
                g->soft_drop_frames++;
            }
        }
        else
        {
            next_piece(&(g->piece), &(g->next_piece), &(g->rngstate), g->history, false);
            g->piece_inactive = false;
            g->gcount = 0;
            g->ldcount = g->p.speeds[g->current_speed_index].lock_delay;

            g->soft_drop_frames = 0;

            g->level++;
            if (g->level == g->p.sections[g->current_section_index])
            {
                g->level--;
            }
            else if (g->current_speed_index + 1 < g->p.speeds_length &&
                     g->level >= g->p.speeds[g->current_speed_index + 1].level)
            {
                g->current_speed_index++;
            }

            int rot_amt = 0;
            if (g->curr_keys & KN__A)
            {
                rot_amt = 3;
            }
            else if (g->curr_keys & KN__B)
            {
                rot_amt = 1;
            }
            else if (g->curr_keys & KN__C)
            {
                rot_amt = 3;
            }
            if (rot_amt)
            {
                rotate(g->board, &(g->piece), rot_amt, false);
            }

            while (!check_move(g->board, &(g->piece)))
            {
                if (rot_amt != 0)
                {
                    rotate(g->board, &(g->piece), 4 - rot_amt, false);
                    rot_amt = 0;
                    continue;
                }
                DEBUG(printf("you lose :(\n"));
                g->prog_state = PS__LOSE_SCREEN;
                return;
            }
        }

        if (!is_on_floor(g->board, &(g->piece)))
        {
            if ((g->curr_keys & KN__SONIC_DROP) || (g->curr_keys & KN__HARD_DROP))
            {
                do
                {
                    g->piece.pos.y += 1;
                } while (check_move(g->board, &(g->piece)));
                g->piece.pos.y -= 1;

                if (g->curr_keys & KN__HARD_DROP)
                {
                    lock(g);
                }
                else
                {
                    g->gcount = 0;
                    g->ldcount = g->p.speeds[g->current_speed_index].lock_delay;
                }
            }
            else
            {
                g->gcount += g->p.speeds[g->current_speed_index].gravity;
                if (g->curr_keys & KN__SOFT_DROP && g->p.speeds[g->current_speed_index].gravity < 256)
                {
                    g->gcount += 256 - g->p.speeds[g->current_speed_index].gravity;
                }
                if (g->gcount >= 256)
                {
                    do
                    {
                        g->gcount -= 256;
                        g->piece.pos.y += 1;
                        if (!check_move(g->board, &(g->piece)))
                        {
                            g->piece.pos.y -= 1;
                            // TODO: nail down behavior of when piece moves back and forth over block
                            break;
                        }
                        g->ldcount = g->p.speeds[g->current_speed_index].lock_delay;
                    } while (g->gcount >= 256);
                }
            }
        }
        else
        {
            if ((g->curr_keys & KN__SOFT_DROP) || (g->curr_keys & KN__HARD_DROP))
            {
                g->ldcount = 0;
            }
            if (g->ldcount == 0)
            {
                lock(g);
            }
            else if (g->p.speeds[g->current_speed_index].lock_delay != INF_LOCK_DELAY)
            {
                g->ldcount -= 1;
            }
        }
    }
}

static void fill_rect(u8* frame_buffer, int x, int y, int w, int h, Color c)
{
    y = 240 - y;
    int xw = x + w, yh = y - h;
    for (int i = y; i > yh; i--)
    {
        for (int j = x; j < xw; j++)
        {
            frame_buffer[3 * (i + 240 * j)] = c.b;
            frame_buffer[3 * (i + 240 * j) + 1] = c.g;
            frame_buffer[3 * (i + 240 * j) + 2] = c.r;
        }
    }
}

// x and y specify the top-left corner of the rightmost digit
static void draw_int(u8* frame_buffer, int n, u8 scale, int x, int y, Color c)
{
    y = 240 - y;
    bool negative = n < 0;
    if (negative)
    {
        n = -n;
    }

    for (;;)
    {
        int digit;
        if (n == -1)
        {
            n = 0;
            digit = 10; // minus sign
        }
        else
        {
            digit = n % 10;
        }

        for (int row = 0; row < 5 * scale; row++)
        {
            for (int col = 0; col < 4 * scale; col++)
            {
                if (DIGITS[digit][row / scale][col / scale])
                {
                    frame_buffer[3 * (y - row + 240 * (x + col))] = c.b;
                    frame_buffer[3 * (y - row + 240 * (x + col)) + 1] = c.g;
                    frame_buffer[3 * (y - row + 240 * (x + col)) + 2] = c.r;
                }
            }
        }
        x -= 5 * scale;
        n /= 10;

        if (n == 0)
        {
            if (negative)
            {
                negative = false;
                n = -1;
            }
            else
            {
                break;
            }
        }
    }
}

// x and y specify the top-left corner of the first character of the string
static void draw_str(u8* frame_buffer, const char* str, int len, u8 scale, int x, int y, Color c)
{
    y = 240 - y;
    for (int i = 0; i < len; i++)
    {
        for (int row = 0; row < 6 * scale; row++)
        {
            for (int col = 0; col < 5 * scale; col++)
            {
                if (FONT[str[i] - 32][row / scale][col / scale])
                {
                    frame_buffer[3 * (y - row + 240 * (x + col))] = c.b;
                    frame_buffer[3 * (y - row + 240 * (x + col)) + 1] = c.g;
                    frame_buffer[3 * (y - row + 240 * (x + col)) + 2] = c.r;
                }
            }
        }
        x += 6 * scale;
    }
}

static inline void game_render(Game* g, u8* frame_buffer)
{
    memset(frame_buffer, 0, 240 * 400 * 3);

    Color c;
    c.b = 0x20;
    c.g = 0x20;
    c.r = 0x20;
    fill_rect(frame_buffer, 149, 20, 102, 202, c);

    int bar_width;
    if (g->piece_inactive)
    {
        c = TETROMINO_COLORS[0];
        bar_width = 102.0 * ((double)g->arecount / g->p.speeds[g->current_speed_index].line_are);
    }
    else if (is_on_floor(g->board, &(g->piece)))
    {
        c = TETROMINO_COLORS[3];
        bar_width = 102.0 * ((double)g->ldcount / g->p.speeds[g->current_speed_index].lock_delay);
    }
    else
    {
        c = TETROMINO_COLORS[5];
        bar_width = 102.0 * (1.0 - (double)g->gcount / 256.0);
    }
    if (bar_width > 102.0)
    {
        bar_width = 102.0;
    }
    fill_rect(frame_buffer, 149, 223, bar_width, 10, c);

    for (int row = 2; row < BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < BOARD_WIDTH; col++)
        {
            //DEBUG(printf("%d   %d", row, col));
            if (g->board[row][col] != 0)
            {
                //DEBUG(printf("%d", g->board[row][col]));
                ASSERT(g->board[row][col] > 0, " ");
                fill_rect(frame_buffer, 151 + 10 * col, 2 + 10 * row,
                          8, 8, TETROMINO_COLORS[g->board[row][col] - 1]);
            }
        }
    }

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (!g->piece_inactive && TETROMINOS[g->piece.tet][g->piece.rot][row][col] != 0
                && (row + g->piece.pos.y) >= 2)
            {
                fill_rect(frame_buffer, 151 + 10 * (col + g->piece.pos.x), 2 + 10 * (row + g->piece.pos.y),
                          8, 8, TETROMINO_COLORS[g->piece.tet]);
            }

            if (TETROMINOS[g->next_piece.tet][0][row][col] != 0)
            {
                fill_rect(frame_buffer, 300 + 10 * col, 20 + 10 * row,
                          8, 8, TETROMINO_COLORS[g->next_piece.tet]);
            }

/*            DEBUG(
              for (int i = 0; i < HISTORY_LENGTH; i++)
              {
              if (TETROMINOS[g->history[i]][0][row][col] != 0)
              {
              fill_rect(frame_buffer, 50 + 10 * col, 10 + 10 * row + 50 * i,
              8, 8, TETROMINO_COLORS[g->history[i]]);
              }
              }
              );*/
        }
    }

    c.b = 0x05;
    c.g = 0x20;
    c.r = 0x05;
    fill_rect(frame_buffer, 85, 210, 38, 16, c);
    if (g->dascount < g->p.speeds[g->current_speed_index].das)
    {
        c.g = 0x50;
    }
    else
    {
        c.g = 0xE0;
    }
    draw_str(frame_buffer, "DAS", 3, 2, 87, 212, c);


    c.b = 0xFF;
    c.g = 0xFF;
    c.r = 0xFF;
    draw_int(frame_buffer, g->level, 3, 320, 100, c);
    draw_int(frame_buffer, g->p.sections[g->current_section_index], 3, 320, 130, c);

    u8 grade = current_grade(g);
    draw_str(frame_buffer, GRADE_STRINGS[grade], 2, 3, 40, 40, c);

    u8 csecs, seconds, minutes;
    if (g->timer > 0)
    {
        csecs = ((g->timer * 100) / 60) % 100;
        seconds = (g->timer / 60) % 60;
        minutes = g->timer / (60 * 60);
    }
    else
    {
        csecs = 0;
        seconds = 0;
        minutes = 0;
    }
    char temp_str[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    sprintf(temp_str, "%02hhd:%02hhd:%02hhd", minutes, seconds, csecs);
    draw_str(frame_buffer, temp_str, strlen(temp_str), 2, 30, 100, c);

    draw_str(frame_buffer, "NEXT: ", 6, 1, 30, 160, c);
    memset(temp_str, 0, sizeof(temp_str));
    if (GRADE_SCORES[grade + 1] == GRADE_SCORES__TRIDASH)
    {
        sprintf(temp_str, "---");
    }
    else if (GRADE_SCORES[grade + 1] == GRADE_SCORES__HEXMARK)
    {
        sprintf(temp_str, "??????");
    }
    else
    {
        sprintf(temp_str, "%lu", GRADE_SCORES[grade + 1]);
    }
    u8 l = strlen(temp_str);  // can you get this out of sprintf?
    draw_str(frame_buffer, temp_str, l, 1, 120 - 6 * l, 160, c);

    memset(temp_str, 0, sizeof(temp_str));
    sprintf(temp_str, "%lu", g->score);
    l = strlen(temp_str);
    if (g->gm_eligible)
    {
        c.g = 0xD0;
        c.b = 0x80;
    }
    draw_str(frame_buffer, temp_str, l, 1, 120 - 6 * l, 150, c);
}


static inline void menu_update(Game *g, u32 k_down)
{
    if (k_down & KEY_START)
    {
        game_reset(g);
        g->prog_state = PS__IN_GAME;
    }
    else if (k_down & (KEY_A | KEY_DLEFT | KEY_DRIGHT))
    {
        g->menu.item_callbacks[g->menu.cursor_pos](g, k_down, g->menu.cursor_pos);
    }
    else if (k_down & KEY_DUP)
    {
        g->menu.cursor_pos += NUM_MENU_ITEMS;
        g->menu.cursor_pos--;
        g->menu.cursor_pos %= NUM_MENU_ITEMS;
    }
    else if (k_down & KEY_DDOWN)
    {
        g->menu.cursor_pos++;
        g->menu.cursor_pos %= NUM_MENU_ITEMS;
    }
    else if (k_down & KEY_SELECT)
    {
        g->prog_state = PS__QUITTING;
    }


    /* if (hidKeysHeld() & KEY_L) */
    /*     g->inf_lock_delay = true; */
}

static inline void menu_render(Game *g, u8 *frame_buffer)
{
    memset(frame_buffer, 0, 240 * 400 * 3);

    Color c = {0xFF, 0xFF, 0xFF};

    for (int i = 0; i < NUM_MENU_ITEMS; i++)
    {
        // TODO: perhaps replace the strlen, it's kind of wasted effort
        draw_str(frame_buffer, g->menu.item_strs[i], strlen(g->menu.item_strs[i]), 2, 80, 20 + 16 * i, c);
    }

    fill_rect(frame_buffer, 69, 24 + 16 * g->menu.cursor_pos, 5, 5, c);
}


static inline void input_update_render(Game *g, u8 *frame_buffer)
{
    u32 k_down = hidKeysDown();

    switch (g->prog_state)
    {
    case PS__IN_GAME:
        game_process_input(g);
        game_update(g);
        game_render(g, frame_buffer);

        if (k_down & KEY_START)
        {
            g->prog_state = PS__PAUSED;
        }
        break;

    case PS__PAUSED:
        game_render(g, frame_buffer);
        fill_rect(frame_buffer, 149, 20, 102, 202, (Color){0x20, 0x20, 0x20});
        fill_rect(frame_buffer, 151, 115, 99, 25, (Color){0x00, 0x00, 0x00});
        draw_str(frame_buffer, " PAUSED", 7, 2, 153, 120, (Color){0xFF, 0xFF, 0xFF});

        if (k_down & (KEY_START | KEY_A | KEY_B))
        {
            g->prog_state = PS__IN_GAME;
        }
        else if (k_down & KEY_SELECT)
        {
            g->prog_state = PS__LOSE_SCREEN;
        }
        break;

    case PS__LOSE_SCREEN:
        game_render(g, frame_buffer);
        fill_rect(frame_buffer, 151, 115, 99, 25, (Color){0x00, 0x00, 0x00});
        draw_str(frame_buffer, "YOU LOSE", 8, 2, 153, 120, (Color){0x00, 0x00, 0xFF});

        if (k_down & (KEY_START | KEY_A))
        {
            game_reset(g);
            g->prog_state = PS__IN_GAME;
        }
        else if (k_down & (KEY_SELECT | KEY_B))
        {
            g->prog_state = PS__MAIN_MENU;
        }
        break;

    case PS__MAIN_MENU:
        menu_update(g, k_down);
        menu_render(g, frame_buffer);
        break;

    case PS__QUITTING:
        // intentionally left blank, quitting is handled in the main loop
        break;

    default:
        ASSERT(0, "prog_state is an invalid value %d", g->prog_state);
    }
}




int main()
{
    gfxInitDefault();

    consoleInit(GFX_BOTTOM, NULL);

    Game g;
    game_init(&g);
    g.prog_state = PS__MAIN_MENU;

    int frame_count = 0;

    while (aptMainLoop())
    {
        gspWaitForVBlank();
        hidScanInput();

        // 240x400 and 240x320
        u8* frame_buffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

        frame_count++;
        /* if (frame_count % 60 == 0) */
        /* { */
        /*     if (frame_count % 120 == 0) */
        /*         printf("tock   %d\n", frame_count); */
        /*     else */
        /*         printf("tick   %d\n", frame_count); */
        /* } */

        input_update_render(&g, frame_buffer);

        if (g.prog_state == PS__QUITTING)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    gfxExit();
    DEBUG(printf("ending...\n"));

    return 0;
}


/*
 * TODO
 * frame-step debug button
 * button remapping (inc multiple keys per same button)
 * an actual menu???
 */
