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
#define GRADE_SCORES__TRIMARK ((u32)-1)
#define GRADE_SCORES__TRIDASH ((u32)-2)
const u32 GRADE_SCORES[NUM_GRADES + 1] =
{
    0, 400, 800, 1400, 2000, 3500, 5500, 8000, 12000,
    16000, 22000, 30000, 40000, 52000, 66000, 82000, 100000, 120000,
    GRADE_SCORES__TRIMARK, GRADE_SCORES__TRIDASH
};

typedef struct
{
    int tet;
    Position pos;
    int rot;
} Piece;

typedef enum
{
    MS__IN_GAME,
    MS__PAUSED,
    MS__LOSE_SCREEN,
    MS__MAIN_MENU,
    MS__QUITTING,
} MenuState;

typedef struct
{
    MenuState menu_state;
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

    int level;
    int sections[MAX_SECTIONS_LENGTH];
    int sections_length;
    int current_section_index;

    u32 score;
    u8 combo;
    u8 soft_drop_frames;
    bool gm_eligible;

    u8 gravity;
    u8 gravity_multiplier;
    u8 gcount;
    u8 lock_delay;
    bool inf_lock_delay;
    u8 ldcount;
    u8 line_are;
    u8 are;
    u8 arecount;
    u8 das;
    u8 dascount;
    int das_direction;
} Game;


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
    int row, col;

    *bravo = 4;

    for (row = 2; row < BOARD_HEIGHT; row++)
    {
        bool line = true;
        for (col = 0; col < BOARD_WIDTH; col++)
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
            for (col = 0; col < BOARD_WIDTH; col++)
            {
                board[row][col] = 0;
            }
        }
    }
    return line_count;
}

static void cascade(s8 board[BOARD_HEIGHT][BOARD_WIDTH])
{
    int row, col;
    // TODO: should this start at 2
    for (row = 2; row < BOARD_HEIGHT; row++)
    {
        bool empty = true;
        for (col = 0; col < BOARD_WIDTH; col++)
        {
            if (board[row][col] != 0) {
                empty = false;
                //break; //TODO: is this ok
            }
        }

        if (empty)
        {
            int row2;
            //TODO maybe make a little nicer once everything confirmed working
            for (row2 = row - 1; row2 >= 0; row2--)
            {
                for (col = 0; col < BOARD_WIDTH; col++)
                {
                    board[row2 + 1][col] = board[row2][col];
                }
            }
            for (col = 0; col < BOARD_WIDTH; col++)
            {
                board[0][col] = 0;
            }
        }
    }
}

static bool check_move(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece)
{
    int row, col;
    for (row = 0; row < 4; row++)
    {
        for (col = 0; col < 4; col++)
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

static inline void horiz_move(s8 board[BOARD_HEIGHT][BOARD_WIDTH], Piece* piece, int amt)
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
    u64 i;
    for (i = 2ULL; i < (1ULL << 63ULL); i <<= 1ULL)
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

    int i, j;
    bool found;
    for (i = 0; i < HISTORY_TRIES; i++)
    {
        found = false;
        new_piece->tet = next_piece__randpiece(state);
        //DEBUG(printf("generated piece %d, ", new_piece->tet));
        for (j = 0; j < HISTORY_LENGTH; j++)
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

    for (i = HISTORY_LENGTH - 1; i > 0; i--)
    {
        history[i] = history[i - 1];
    }
    history[0] = new_piece->tet;

    ASSERT(new_piece->tet < 7, "piece out of bounds (after history check): %d", new_piece->tet);
}

static inline void lock(Game* g)
{
    int row, col;
    for (row = 0; row < 4; row++)
    {
        for (col = 0; col < 4; col++)
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
        g->arecount = g->are;
        g->combo = 1;
    }
    else
    {
        g->arecount = g->line_are;
        ASSERT(lines_cleared > 0 && lines_cleared < 5, "%hhd", lines_cleared);

        // https://tetris.wiki/Tetris_The_Grand_Master#Scoring
        // note that combo should be updated after calculating score, not before
        // the page (as of 3/22/16) is wrong about this
        g->score += ((g->level + lines_cleared - 1) / 4 + 1 + g->soft_drop_frames)
            * lines_cleared * (2 * lines_cleared - 1) * g->combo * bravo;
        DEBUG(printf("level: %hhu, lines: %hhu, sdf: %hhu, combo: %hhu, bravo: %hhu\n",
                     g->level, lines_cleared, g->soft_drop_frames, g->combo, bravo));
        g->combo += 2 * lines_cleared - 2;

        g->level += lines_cleared;
        if (g->level >= g->sections[g->current_section_index]
            && g->current_section_index < g->sections_length - 1)
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


static inline void game_init(Game* g)
{
    g->menu_state = MS__MAIN_MENU;

    //g->rngstate = 1311666606263354484;
    g->rngstate = osGetTime();
    DEBUG(printf("%llu\n", g->rngstate));
    int i;
    for (i = 0; i < 100; i++) next_piece__randpiece(&(g->rngstate));
    DEBUG(printf("%llu\n", g->rngstate));

    g->timer = -60; // to reach zero when piece actually starts

    g->curr_keys = 0;
    g->old_keys = 0;

    _Static_assert(NUM_KEY_NAMES == 8, "need to update key config");
    g->key_config[0 /*KN__SONIC_DROP*/] = KEY_DUP;
    g->key_config[1 /*KN__HARD_DROP*/] = KEY_NONE;
    g->key_config[2 /*KN__SOFT_DROP*/] = KEY_DDOWN;
    g->key_config[3 /*KN__LEFT*/] = KEY_DLEFT;
    g->key_config[4 /*KN__RIGHT*/] = KEY_DRIGHT;
    g->key_config[5 /*KN__A*/] = KEY_B;
    g->key_config[6 /*KN__B*/] = KEY_A;
    g->key_config[7 /*KN__C*/] = KEY_NONE;

    memset(g->board, 0, BOARD_HEIGHT * BOARD_WIDTH);

    _Static_assert(HISTORY_LENGTH == 4, "need to update history initialization");
    g->history[0] = PIECE__Z;
    g->history[1] = PIECE__Z;
    g->history[2] = PIECE__Z;
    g->history[3] = PIECE__Z;

/*    g->piece.tet = 0;
      g->piece.pos.x = 3;
      g->piece.pos.y = 1;
      g->piece.rot = 0;

      g->next_piece.tet = 1;
      g->next_piece.pos.x = 3;
      g->next_piece.pos.y = 1;
      g->next_piece.rot = 0;*/
    next_piece(&(g->piece), &(g->next_piece), &(g->rngstate), g->history, true);
    //next_piece(&(g->piece), &(g->next_piece), &(g->rngstate), g->history, false);

    g->piece_inactive = true;

    g->level = 0;
    for (i = 0; i < 9; i++) g->sections[i] = (i + 1) * 100;
    g->sections[9] = 999;
    g->sections_length = 10;
    g->current_section_index = 0;

    g->score = 0;
    g->combo = 1;
    g->soft_drop_frames = 0;
    g->gm_eligible = true;

    g->gravity = 0;
    g->gcount = g->gcount;
    g->gravity_multiplier = 20;

    g->lock_delay = 30;
    g->inf_lock_delay = false;
    g->ldcount = g->lock_delay;

    g->line_are = 41;
    g->are = 30;
    g->arecount = -1 * g->timer;

    g->das = 14;
    g->dascount = 0;
    g->das_direction = 0;
}

static inline void game_process_input(Game* g)
{
    u32 k_held = hidKeysHeld();

    g->old_keys = g->curr_keys;

    u32 i;
    for (i = 0; i < NUM_KEY_NAMES; i++)
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

    if (hidKeysDown() & KEY_START)
    {
        g->menu_state = MS__PAUSED;
    }
}

static inline void game_update(Game* g)
{
    g->timer++;

    if (g->curr_keys & KN__LEFT)
    {
        if (g->das_direction == -1)
        {
            if (g->dascount < g->das)
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
            if (g->dascount < g->das)
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
        if (g->arecount == g->line_are / 2)
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
            if (g->dascount >= g->das)
            {
                ASSERT(g->das == g->dascount, " ");
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
            g->gcount = g->gravity;
            g->ldcount = g->lock_delay;

            g->soft_drop_frames = 0;

            g->level++;
            if (g->level == g->sections[g->current_section_index])
            {
                g->level--;
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
                g->menu_state = MS__LOSE_SCREEN;
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
                    g->gcount = g->gravity;
                    g->ldcount = g->lock_delay;
                }
            }
            else
            {
                if (g->gcount == 0 || (g->curr_keys & KN__SOFT_DROP))
                {
                    g->gcount = g->gravity;
                    g->ldcount = g->lock_delay;
                    g->piece.pos.y += 1;
                    int i;
                    for (i = 1; i < g->gravity_multiplier; i++)
                    {
                        g->piece.pos.y += 1;
                        if (!check_move(g->board, &(g->piece)))
                        {
                            g->piece.pos.y -= 1;
                            break;
                        }
                    }
                }
                else
                {
                    g->gcount -= 1;
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
            else if (!g->inf_lock_delay)
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
    int i, j;
    for (i = y; i > yh; i--)
    {
        for (j = x; j < xw; j++)
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
        int row, col, digit;
        if (n == -1)
        {
            n = 0;
            digit = 10; // minus sign
        }
        else
        {
            digit = n % 10;
        }
        for (row = 0; row < 5 * scale; row++)
        {
            for (col = 0; col < 4 * scale; col++)
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
    int row, col, i;
    for (i = 0; i < len; i++)
    {
        for (row = 0; row < 6 * scale; row++)
        {
            for (col = 0; col < 5 * scale; col++)
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
        bar_width = 102.0 * ((double)g->arecount / g->line_are);
    }
    else if (is_on_floor(g->board, &(g->piece)))
    {
        c = TETROMINO_COLORS[3];
        bar_width = 102.0 * ((double)g->ldcount / g->lock_delay);
    }
    else
    {
        c = TETROMINO_COLORS[5];
        bar_width = 102.0 * ((double)g->gcount / g->gravity);
    }
    fill_rect(frame_buffer, 149, 223, bar_width, 10, c);

    int row, col;
    for (row = 2; row < BOARD_HEIGHT; row++)
    {
        for (col = 0; col < BOARD_WIDTH; col++)
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

    for (row = 0; row < 4; row++)
    {
        for (col = 0; col < 4; col++)
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
              int i;
              for (i = 0; i < HISTORY_LENGTH; i++)
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
    if (g->dascount < g->das)
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
    draw_int(frame_buffer, g->sections[g->current_section_index], 3, 320, 130, c);

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
    if (GRADE_SCORES[grade] == GRADE_SCORES__TRIDASH)
    {
        sprintf(temp_str, "---");
    }
    else if (GRADE_SCORES[grade] == GRADE_SCORES__TRIMARK)
    {
        sprintf(temp_str, "???");
    }
    else
    {
        sprintf(temp_str, "%lu", GRADE_SCORES[grade + 1]);
    }
    u8 l = strlen(temp_str);
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


static inline void input_update_render(Game* g, u8* frame_buffer)
{
    u32 k_down = hidKeysDown();

    switch (g->menu_state)
    {
    case MS__IN_GAME:
        game_process_input(g);
        game_update(g);
        game_render(g, frame_buffer);
        break;

    case MS__PAUSED:
        game_render(g, frame_buffer);
        fill_rect(frame_buffer, 149, 20, 102, 202, (Color){0x20, 0x20, 0x20});
        fill_rect(frame_buffer, 151, 115, 99, 25, (Color){0x00, 0x00, 0x00});
        draw_str(frame_buffer, " PAUSED", 7, 2, 153, 120, (Color){0xFF, 0xFF, 0xFF});

        if (k_down & (KEY_START | KEY_A | KEY_B))
        {
            g->menu_state = MS__IN_GAME;
        }
        else if (k_down & KEY_SELECT)
        {
            g->menu_state = MS__LOSE_SCREEN;
        }
        break;

    case MS__LOSE_SCREEN:
        game_render(g, frame_buffer);
        fill_rect(frame_buffer, 151, 115, 99, 25, (Color){0x00, 0x00, 0x00});
        draw_str(frame_buffer, "YOU LOSE", 8, 2, 153, 120, (Color){0x00, 0x00, 0xFF});

        if (k_down & (KEY_START | KEY_A | KEY_B))
        {
            game_init(g);
            if (hidKeysHeld() & KEY_L)
                g->inf_lock_delay = true;
            g->menu_state = MS__IN_GAME;
        }
        else if (k_down & KEY_SELECT)
        {
            g->menu_state = MS__QUITTING;
        }
        break;

    case MS__MAIN_MENU:
        //memset(frame_buffer, 0, 240 * 400 * 3);
        ASSERT(0, "MS__MAIN_MENU incomplete");
        break;

    case MS__QUITTING:
        // intentionally left blank, quitting is handled in the main loop
        break;

    default:
        ASSERT(0, "menu_state is an invalid value %d", g->menu_state);
    }
}




int main()
{
    gfxInitDefault();

    consoleInit(GFX_BOTTOM, NULL);

    Game g;
    game_init(&g);
    g.menu_state = MS__LOSE_SCREEN;

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

        if (g.menu_state == MS__QUITTING)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    gfxExit();
    DEBUG(printf("ending...\n"));

    return 0;
}
