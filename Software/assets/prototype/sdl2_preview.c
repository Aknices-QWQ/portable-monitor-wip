#include <SDL2/SDL.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOGICAL_W 1920
#define LOGICAL_H 1280
#define WINDOW_W  1600
#define WINDOW_H  1066

#define TOP_H     62
#define BOTTOM_H  50
#define PARAM_W   363
#define WAVE_W    (LOGICAL_W - PARAM_W)
#define ROWS      5
#define FONT_W    5
#define FONT_H    7

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Color;

static const Color BG          = {0x04, 0x07, 0x04, 0xff};
static const Color TOP_BG      = {0x08, 0x0e, 0x08, 0xff};
static const Color BORDER      = {0x0e, 0x1e, 0x0e, 0xff};
static const Color ROW_BORDER  = {0x0b, 0x17, 0x0b, 0xff};
static const Color GRID_MAJOR  = {0x0d, 0x24, 0x10, 0xff};
static const Color GRID_MINOR  = {0x09, 0x14, 0x0a, 0xff};
static const Color GREEN1      = {0x00, 0xff, 0x88, 0xff};
static const Color GREEN2      = {0x00, 0xe4, 0x7a, 0xff};
static const Color GREEN3      = {0x00, 0xc8, 0x68, 0xff};
static const Color GREEN4      = {0x00, 0xac, 0x58, 0xff};
static const Color CYAN        = {0x00, 0xcc, 0xff, 0xff};
static const Color AMBER       = {0xff, 0xaa, 0x44, 0xff};
static const Color BLUE        = {0x00, 0x88, 0xbb, 0xff};
static const Color TEXT_DIM    = {0x2a, 0x5a, 0x2a, 0xff};
static const Color TEXT_MID    = {0x3a, 0x6a, 0x3a, 0xff};
static const Color STATUS_BG   = {0x05, 0x0b, 0x05, 0xff};
static const Color PANEL_HR_BG = {0x06, 0x0c, 0x06, 0xff};
static const Color PANEL_NI_BG = {0x06, 0x07, 0x0b, 0xff};
static const Color PANEL_OX_BG = {0x04, 0x09, 0x0c, 0xff};
static const Color BADGE_BG    = {0x0c, 0x1e, 0x0c, 0xff};
static const Color ALARM_BG    = {0x04, 0x10, 0x04, 0xff};

static const Color LEAD_COLORS[ROWS] = {
    {0x00, 0xff, 0x88, 0xff},
    {0x00, 0xe4, 0x7a, 0xff},
    {0x00, 0xc8, 0x68, 0xff},
    {0x00, 0xac, 0x58, 0xff},
    {0x00, 0xcc, 0xff, 0xff},
};

static float ecg_wave_sample(float seconds, int lead_index)
{
    float p = fmodf(seconds * 1.2f, 1.0f);
    float v = 0.0f;
    static const float lead_gain[4] = { 1.00f, 0.86f, 0.56f, -0.42f };

    if(p < 0.05f) v = sinf((p / 0.05f) * (float)M_PI) * 0.08f;
    else if(p < 0.12f) v = -sinf(((p - 0.05f) / 0.07f) * (float)M_PI) * 0.04f;
    else if(p < 0.18f) v = sinf(((p - 0.16f) / 0.02f) * (float)M_PI) * 0.14f;
    else if(p < 0.20f) v = -sinf(((p - 0.18f) / 0.02f) * (float)M_PI) * 0.07f;
    else if(p < 0.22f) v = -sinf(((p - 0.20f) / 0.02f) * (float)M_PI) * 0.18f;
    else if(p < 0.24f) v = sinf(((p - 0.22f) / 0.02f) * (float)M_PI) * 1.00f;
    else if(p < 0.26f) v = sinf(((p - 0.24f) / 0.02f) * (float)M_PI) * -0.28f;
    else if(p < 0.30f) v = -sinf(((p - 0.26f) / 0.04f) * (float)M_PI) * 0.05f;
    else if(p < 0.46f) v = sinf(((p - 0.36f) / 0.10f) * (float)M_PI) * 0.16f;

    return v * lead_gain[lead_index % 4];
}

static float spo2_wave_sample(float seconds)
{
    float p = fmodf(seconds * 1.2f, 1.0f);

    if(p < 0.15f) return sinf((p / 0.15f) * (float)M_PI) * 0.90f;
    if(p < 0.25f) return sinf(((p - 0.15f) / 0.10f) * (float)M_PI) * -0.14f;
    return 0.0f;
}

static const Uint8 *glyph_for_char(char ch)
{
    static const Uint8 unknown[FONT_H] = {0x0e, 0x11, 0x02, 0x04, 0x04, 0x00, 0x04};
    static const Uint8 space[FONT_H]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static const Uint8 dash[FONT_H]    = {0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00};
    static const Uint8 dot[FONT_H]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c};
    static const Uint8 colon[FONT_H]   = {0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00};
    static const Uint8 slash[FONT_H]   = {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00};
    static const Uint8 plus[FONT_H]    = {0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00};
    static const Uint8 pct[FONT_H]     = {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13};

    static const Uint8 digits[10][FONT_H] = {
        {0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e},
        {0x04, 0x0c, 0x14, 0x04, 0x04, 0x04, 0x1f},
        {0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f},
        {0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e},
        {0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02},
        {0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e},
        {0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e},
        {0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
        {0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e},
        {0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x1c},
    };

    static const Uint8 letters[26][FONT_H] = {
        {0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}, /* A */
        {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e}, /* B */
        {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e}, /* C */
        {0x1c, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1c}, /* D */
        {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f}, /* E */
        {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10}, /* F */
        {0x0f, 0x10, 0x10, 0x13, 0x11, 0x11, 0x0f}, /* G */
        {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}, /* H */
        {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e}, /* I */
        {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0e}, /* J */
        {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, /* K */
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f}, /* L */
        {0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11}, /* M */
        {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, /* N */
        {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}, /* O */
        {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10}, /* P */
        {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d}, /* Q */
        {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11}, /* R */
        {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e}, /* S */
        {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, /* T */
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}, /* U */
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04}, /* V */
        {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a}, /* W */
        {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11}, /* X */
        {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04}, /* Y */
        {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f}, /* Z */
    };

    if(ch >= '0' && ch <= '9') return digits[ch - '0'];
    if(ch >= 'A' && ch <= 'Z') return letters[ch - 'A'];

    switch(ch) {
    case ' ': return space;
    case '-': return dash;
    case '.': return dot;
    case ':': return colon;
    case '/': return slash;
    case '+': return plus;
    case '%': return pct;
    default: return unknown;
    }
}

static void set_color(SDL_Renderer *renderer, Color c)
{
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

static void fill_rect(SDL_Renderer *renderer, int x, int y, int w, int h, Color c)
{
    SDL_Rect rect = { x, y, w, h };
    set_color(renderer, c);
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_hline(SDL_Renderer *renderer, int x1, int x2, int y, Color c)
{
    set_color(renderer, c);
    SDL_RenderDrawLine(renderer, x1, y, x2, y);
}

static void draw_vline(SDL_Renderer *renderer, int x, int y1, int y2, Color c)
{
    set_color(renderer, c);
    SDL_RenderDrawLine(renderer, x, y1, x, y2);
}

static void draw_text(SDL_Renderer *renderer, int x, int y, int scale, Color c, const char *text)
{
    int cursor_x = x;
    size_t i;

    set_color(renderer, c);

    for(i = 0; text[i] != '\0'; ++i) {
        Uint8 row;
        char ch = (char)toupper((unsigned char)text[i]);
        const Uint8 *glyph = glyph_for_char(ch);

        for(row = 0; row < FONT_H; ++row) {
            Uint8 col;
            for(col = 0; col < FONT_W; ++col) {
                if(glyph[row] & (1U << (FONT_W - 1U - col))) {
                    SDL_Rect px = {
                        cursor_x + col * scale,
                        y + row * scale,
                        scale,
                        scale
                    };
                    SDL_RenderFillRect(renderer, &px);
                }
            }
        }

        cursor_x += (FONT_W + 1) * scale;
    }
}

static void draw_badge(SDL_Renderer *renderer, int x, int y, int w, int h, Color bg, Color border, Color text, const char *label)
{
    fill_rect(renderer, x, y, w, h, bg);
    draw_hline(renderer, x, x + w - 1, y, border);
    draw_hline(renderer, x, x + w - 1, y + h - 1, border);
    draw_vline(renderer, x, y, y + h - 1, border);
    draw_vline(renderer, x + w - 1, y, y + h - 1, border);
    draw_text(renderer, x + 8, y + 7, 2, text, label);
}

static void draw_battery(SDL_Renderer *renderer, int x, int y, int pct)
{
    int i;
    int bars = (pct + 24) / 25;

    fill_rect(renderer, x, y + 6, 28, 14, TEXT_DIM);
    fill_rect(renderer, x + 28, y + 10, 4, 6, TEXT_DIM);
    fill_rect(renderer, x + 2, y + 8, 24, 10, TOP_BG);

    for(i = 0; i < bars; ++i) {
        fill_rect(renderer, x + 4 + i * 5, y + 10, 4, 6, TEXT_MID);
    }

    draw_text(renderer, x + 42, y + 6, 2, TEXT_DIM, "84%");
}

static void draw_wave_panel(SDL_Renderer *renderer, float seconds)
{
    int main_y1 = TOP_H;
    int main_y2 = LOGICAL_H - BOTTOM_H;
    int row_h = (main_y2 - main_y1) / ROWS;
    const char *labels[ROWS] = { "I", "II", "III", "aVR", "SpO2" };

    for(int x = 0; x < WAVE_W; x += 40) draw_vline(renderer, x, main_y1, main_y2, GRID_MAJOR);
    for(int y = main_y1; y < main_y2; y += 40) draw_hline(renderer, 0, WAVE_W, y, GRID_MAJOR);
    for(int x = 0; x < WAVE_W; x += 8) draw_vline(renderer, x, main_y1, main_y2, GRID_MINOR);
    for(int y = main_y1; y < main_y2; y += 8) draw_hline(renderer, 0, WAVE_W, y, GRID_MINOR);

    for(int i = 0; i < ROWS; ++i) {
        int y1 = main_y1 + i * row_h;
        int y2 = (i == ROWS - 1) ? main_y2 : (y1 + row_h);
        int mid = (y1 + y2) / 2;
        int amp = (int)((y2 - y1) * 0.34f);
        int prev_x = 0;
        int prev_y = mid;

        if(i < ROWS - 1) draw_hline(renderer, 0, WAVE_W, y2 - 1, ROW_BORDER);

        draw_text(renderer, 10, y1 + 10, 3, LEAD_COLORS[i], labels[i]);

        set_color(renderer, LEAD_COLORS[i]);
        for(int x = 0; x < WAVE_W; ++x) {
            float t = seconds + (float)x / 170.0f;
            float v = (i == ROWS - 1) ? spo2_wave_sample(t) : ecg_wave_sample(t, i);
            int y = mid - (int)(v * (float)amp);
            if(x > 0) SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);
            prev_x = x;
            prev_y = y;
        }
    }

    (void)labels;
}

static void draw_ui(SDL_Renderer *renderer, float seconds)
{
    int main_y1 = TOP_H;
    int main_y2 = LOGICAL_H - BOTTOM_H;
    int block_h = (main_y2 - main_y1) / 4;
    time_t now = time(NULL);
    struct tm *local_tm = localtime(&now);
    char clock_buf[16] = "00:00:00";

    if(local_tm) {
        snprintf(clock_buf, sizeof(clock_buf), "%02d:%02d:%02d",
                 local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec);
    }

    fill_rect(renderer, 0, 0, LOGICAL_W, LOGICAL_H, BG);

    fill_rect(renderer, 0, 0, LOGICAL_W, TOP_H, TOP_BG);
    draw_hline(renderer, 0, LOGICAL_W, TOP_H - 1, BORDER);

    draw_text(renderer, 18, 18, 2, TEXT_MID, "ECG-5L PORTAMON");
    draw_badge(renderer, 222, 15, 150, 30, BADGE_BG, BORDER, GREEN1, "5-LEAD+RL");
    fill_rect(renderer, 362, 22, 14, 14, GREEN1);
    draw_badge(renderer, 1445, 15, 90, 30, ALARM_BG, BORDER, GREEN3, "HR OK");
    draw_badge(renderer, 1550, 15, 110, 30, ALARM_BG, BORDER, GREEN3, "SPO2 OK");
    draw_text(renderer, 1710, 20, 2, TEXT_DIM, clock_buf);
    draw_battery(renderer, 1810, 14, 84);

    draw_wave_panel(renderer, seconds);
    draw_text(renderer, 1248, 1178, 2, TEXT_DIM, "KNOB ADJ  A/B OK/CANCEL  F1-F4 FUNC");

    fill_rect(renderer, WAVE_W, main_y1, PARAM_W, main_y2 - main_y1, PANEL_HR_BG);
    draw_vline(renderer, WAVE_W, main_y1, main_y2, BORDER);

    fill_rect(renderer, WAVE_W, main_y1, PARAM_W, block_h, PANEL_HR_BG);
    fill_rect(renderer, WAVE_W, main_y1 + block_h, PARAM_W, block_h, PANEL_NI_BG);
    fill_rect(renderer, WAVE_W, main_y1 + block_h * 2, PARAM_W, block_h, PANEL_OX_BG);
    fill_rect(renderer, WAVE_W, main_y1 + block_h * 3, PARAM_W, main_y2 - (main_y1 + block_h * 3), PANEL_OX_BG);

    draw_hline(renderer, WAVE_W, LOGICAL_W, main_y1 + block_h, BORDER);
    draw_hline(renderer, WAVE_W, LOGICAL_W, main_y1 + block_h * 2, BORDER);
    draw_hline(renderer, WAVE_W, LOGICAL_W, main_y1 + block_h * 3, BORDER);

    draw_text(renderer, WAVE_W + 18, main_y1 + 20, 3, GREEN1, "HR");
    draw_text(renderer, WAVE_W + 18, main_y1 + 58, 8, GREEN1, "72");
    draw_text(renderer, WAVE_W + 128, main_y1 + 82, 3, GREEN2, "BPM");
    draw_text(renderer, LOGICAL_W - 80, main_y1 + 16, 2, GREEN1, "100");
    draw_text(renderer, LOGICAL_W - 52, main_y1 + 34, 2, GREEN1, "50");
    draw_text(renderer, WAVE_W + 18, main_y1 + 138, 2, TEXT_MID, "SINUS RHYTHM");
    draw_text(renderer, WAVE_W + 18, main_y1 + 160, 2, GREEN4, "RL DRV");

    draw_text(renderer, WAVE_W + 18, main_y1 + block_h + 20, 3, AMBER, "NIBP");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h + 58, 6, AMBER, "118");
    draw_text(renderer, WAVE_W + 116, main_y1 + block_h + 82, 3, AMBER, "/");
    draw_text(renderer, WAVE_W + 148, main_y1 + block_h + 70, 4, AMBER, "76");
    draw_text(renderer, LOGICAL_W - 80, main_y1 + block_h + 16, 2, AMBER, "160");
    draw_text(renderer, LOGICAL_W - 52, main_y1 + block_h + 34, 2, AMBER, "60");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h + 132, 2, (Color){0xbb, 0x7a, 0x22, 0xff}, "MAP: 90 MMHG");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h + 160, 2, (Color){0x5a, 0x3a, 0x08, 0xff}, "LAST 08:42");

    draw_text(renderer, WAVE_W + 18, main_y1 + block_h * 2 + 20, 3, CYAN, "SPO2");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h * 2 + 58, 8, CYAN, "98");
    draw_text(renderer, WAVE_W + 118, main_y1 + block_h * 2 + 82, 3, (Color){0x00, 0x9d, 0xcc, 0xff}, "%");
    draw_text(renderer, LOGICAL_W - 80, main_y1 + block_h * 2 + 16, 2, CYAN, "100");
    draw_text(renderer, LOGICAL_W - 52, main_y1 + block_h * 2 + 34, 2, CYAN, "90");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h * 2 + 144, 2, (Color){0x6a, 0xa7, 0xba, 0xff}, "PR: 71 BPM");

    draw_text(renderer, WAVE_W + 18, main_y1 + block_h * 3 + 20, 3, BLUE, "PI");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h * 3 + 58, 6, BLUE, "2.8");
    draw_text(renderer, WAVE_W + 108, main_y1 + block_h * 3 + 70, 3, BLUE, "%");
    draw_text(renderer, WAVE_W + 18, main_y1 + block_h * 3 + 140, 2, (Color){0x1a, 0x5a, 0x7a, 0xff}, "PERFUSION");

    fill_rect(renderer, 0, LOGICAL_H - BOTTOM_H, LOGICAL_W, BOTTOM_H, STATUS_BG);
    draw_hline(renderer, 0, LOGICAL_W, LOGICAL_H - BOTTOM_H, BORDER);
    draw_text(renderer, 12, LOGICAL_H - 34, 2, (Color){0x00, 0x88, 0x33, 0xff}, "HR NORMAL");
    draw_text(renderer, 132, LOGICAL_H - 34, 2, (Color){0x00, 0x88, 0x33, 0xff}, "SPO2 NORMAL");
    draw_text(renderer, 270, LOGICAL_H - 34, 2, TEXT_DIM, "NIBP MANUAL");
    draw_text(renderer, 420, LOGICAL_H - 34, 2, TEXT_DIM, "LEADS I II III AVR RL");
    draw_text(renderer, 654, LOGICAL_H - 34, 2, TEXT_DIM, "25MM/S");
    draw_text(renderer, 744, LOGICAL_H - 34, 2, TEXT_DIM, "10MM/MV");
    draw_text(renderer, 850, LOGICAL_H - 34, 2, TEXT_DIM, "MON");
}

int main(int argc, char **argv)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    Uint64 start_ticks;
    bool running = true;
    int result = 1;

    (void)argc;
    (void)argv;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("ECG Monitor SDL2 Preview",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_W,
                              WINDOW_H,
                              SDL_WINDOW_SHOWN);
    if(!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        goto done;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        goto done;
    }

    SDL_RenderSetLogicalSize(renderer, LOGICAL_W, LOGICAL_H);
    SDL_RenderSetIntegerScale(renderer, SDL_FALSE);

    start_ticks = SDL_GetPerformanceCounter();

    while(running) {
        SDL_Event e;
        Uint64 now = SDL_GetPerformanceCounter();
        double freq = (double)SDL_GetPerformanceFrequency();
        float seconds = (float)((double)(now - start_ticks) / freq);

        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) running = false;
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        set_color(renderer, BG);
        SDL_RenderClear(renderer);

        SDL_RenderSetScale(renderer, 1.0f, 1.0f);
        SDL_RenderSetViewport(renderer, NULL);

        draw_ui(renderer, seconds);
        SDL_RenderPresent(renderer);
    }

    result = 0;

done:
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
