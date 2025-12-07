#include "snake.h"
#include "glyphs.h"

#if 0
//FULLSCREEN
#define WINDOW_X 0
#define WINDOW_Y -20
#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1406

#else
//STREAM
/*
#define WINDOW_WIDTH 1770
#define WINDOW_HEIGHT 1405
#define WINDOW_X 10
#define WINDOW_Y -20
*/
#define WINDOW_X 34
#define WINDOW_Y 820
#define WINDOW_WIDTH 1766
#define WINDOW_HEIGHT 1405
#endif

#define GRID_SIZE 56
#define GRID_DIM 1100

#define DELAY 10

void gen_apple();


typedef struct {
	int h;
	int w;
	int gd;

} Win;
Win win;


typedef struct {

    uint8_t *img;
    int img_width;
    int img_height;
    int img_comp;

} Image;
Image logo;


enum {
    SNAKE_UP,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT,
};

typedef struct {
    int f;
    int l;
    int r;
    bool paused;
    bool reward_wall_crash;
    bool reward_apple;
    bool reward_move_to_apple;
    bool reward_tail_crash;

} data;

data Data;

typedef struct {
    int x;
    int y;
    int score;
    int top_score;
} apple;

apple Apple;

struct snake {
    int x;
    int y;
    int dir;
    
    struct snake *next;

};
typedef struct snake Snake;

Snake *head;
Snake *tail;


Snake *g_head;
Snake *g_tail;

void init_ghost(int x, int y, int dir)
{
    Snake *new = malloc(sizeof(Snake));
    new->x = x;
    new->y = y;
    new->dir = dir;
    new->next = NULL;
    
    g_head = new;
    g_tail = new;

    return;
}

void increase_ghost(int x, int y)
{
    Snake *new = malloc(sizeof(Snake));
    new->x = x; 
    new->y = y;
    
    new->next = NULL;
    g_tail->next = new;

    g_tail = new;


    return;
}

void reset_ghost()
{
    Snake *track = g_head;
    Snake *temp;

    while(track != NULL) {
        temp = track;
        track = track->next;
        free(temp);
    }

    g_head = NULL;
    g_tail = NULL;

    return;
}


void SDL_RenderFillCircle(SDL_Renderer *renderer, int x, int y, int radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}


void init_snake()
{
    Snake *new = malloc(sizeof(Snake));
    new->x = rand() % (GRID_SIZE / 2) + (GRID_SIZE / 4);
    new->y = rand() % (GRID_SIZE / 2) + (GRID_SIZE / 4);
    new->dir = SNAKE_UP;
    new->next = NULL;
    
    head = new;
    tail = new;


    return;
}

void increase_snake()
{
    Snake *new = malloc(sizeof(Snake));
    
    switch(tail->dir) {
        case SNAKE_UP:
            new->x = tail->x;
            new->y = tail->y +1;
            break;
        case SNAKE_DOWN:
            new->x = tail->x;
            new->y = tail->y - 1;
            break;
        case SNAKE_LEFT:
            new->x = tail->x + 1;
            new->y = tail->y;
            break;
        case SNAKE_RIGHT:
            new->x = tail->x - 1;
            new->y = tail->y;
            break;
    }

    new->dir = tail->dir;

    new->next = NULL;
    tail->next = new;

    tail = new;


    return;
}

void move_snake()
{

    int prev_x = head->x;
    int prev_y = head->y;
    int prev_dir = head->dir;

    switch(head->dir) {
        case SNAKE_UP:
            head->y--;
            break;
        case SNAKE_DOWN:
            head->y++;
            break;
        case SNAKE_LEFT:
            head->x--;
            break;
        case SNAKE_RIGHT:
            head->x++;
            break;
    }

    Snake *track = head;

    if(track->next != NULL) {
        track = track->next;
    }

    while(track != NULL) {

        int save_x = track->x;
        int save_y = track->y;
        int save_dir = track->dir;

        track->x = prev_x;
        track->y = prev_y;
        track->dir = prev_dir;

        track = track->next;

        prev_x = save_x;
        prev_y = save_y;
        prev_dir = save_dir;

    }


    return;
}


void reset_snake()
{
    Snake *track = head;
    Snake *temp;

    while(track != NULL) {
        temp = track;
        track = track->next;
        free(temp);
    }

    init_snake();
    increase_snake();
    increase_snake();
    increase_snake();

    if(Apple.score > Apple.top_score) {
        Apple.top_score = Apple.score;
    }

    Apple.score = 0;

    gen_apple();

    return;
}

void render_ghost(SDL_Renderer *renderer, int x, int y)
{


    int seg_size = win.gd / GRID_SIZE;
    SDL_Rect seg;
    seg.w = seg_size - 2;
    seg.h = seg_size - 2;

    SDL_Rect seg_out;
    seg_out.w = seg_size;
    seg_out.h = seg_size;

    Snake *track = g_head;

    int bright = 255;
    int b_dir = 0;

    while(track != NULL) {

        SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 55);
        seg_out.x = x + track->x * seg_size;
        seg_out.y = y + track->y* seg_size;

        SDL_RenderDrawRect(renderer, &seg_out);

        SDL_SetRenderDrawColor(renderer, 255 , 0, 0, 25);
        seg.x = x + track->x * seg_size + 1;
        seg.y = y + track->y* seg_size + 1;

        SDL_RenderFillRect(renderer, &seg);

        track = track->next;

        if(b_dir == 0) {
            bright -= 5;
            if(bright < 150) {
                b_dir = 1;
            }
        }
        if(b_dir == 1) {
            bright += 5;
            if(bright > 250) {
                b_dir = 0;
            }
        }
    }

    return;
}

void render_snake(SDL_Renderer *renderer, int x, int y)
{


    int seg_size = win.gd / GRID_SIZE;
    SDL_Rect seg;
    seg.w = seg_size - 2;
    seg.h = seg_size - 2;

    SDL_Rect seg_out;
    seg_out.w = seg_size;
    seg_out.h = seg_size;

    Snake *track = head;

    int bright = 255;
    int b_dir = 0;

    while(track != NULL) {

        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 255);
        seg_out.x = x + track->x * seg_size;
        seg_out.y = y + track->y* seg_size;

        SDL_RenderFillRect(renderer, &seg_out);

        SDL_SetRenderDrawColor(renderer, 0x00, 255 - bright, 0x00, 255);
        seg.x = x + track->x * seg_size + 1;
        seg.y = y + track->y* seg_size + 1;

        SDL_RenderFillRect(renderer, &seg);

        track = track->next;

        if(b_dir == 0) {
            bright -= 5;
            if(bright < 150) {
                b_dir = 1;
            }
        }
        if(b_dir == 1) {
            bright += 5;
            if(bright > 250) {
                b_dir = 0;
            }
        }
    }

    return;
}

void flash_snake(SDL_Renderer *renderer, int x, int y)
{


    int seg_size = win.gd / GRID_SIZE;
    SDL_Rect seg;
    seg.w = seg_size - 2;
    seg.h = seg_size - 2;

    SDL_Rect seg_out;
    seg_out.w = seg_size;
    seg_out.h = seg_size;

    Snake *track = head;

    int bright = 255;
    int b_dir = 0;

    int r = rand() % 255;
    //int g = rand() % 255;
    //int b = rand() % 255;

    int count = 0;

    while(track != NULL) {
        count++;

        SDL_SetRenderDrawColor(renderer, 0x80, 0xe0, 0xff, 255);
        seg_out.x = x + track->x * seg_size;
        seg_out.y = y + track->y* seg_size;

        //SDL_RenderFillRect(renderer, &seg_out);

        SDL_SetRenderDrawColor(renderer, r, r, r, 255);
        seg.x = x + track->x * seg_size + 1;
        seg.y = y + track->y* seg_size + 1;

        if(count % 2 == 0) {
            SDL_RenderFillRect(renderer, &seg);
        }

        track = track->next;

        if(b_dir == 0) {
            bright -= 5;
            if(bright < 150) {
                b_dir = 1;
            }
        }
        if(b_dir == 1) {
            bright += 5;
            if(bright > 250) {
                b_dir = 0;
            }
        }



    }


    return;
}

void render_grid(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x09, 0x09, 0x09, 255);

    int cell_size = win.gd / GRID_SIZE;
#if 1



    SDL_Rect cell;
    cell.w = cell_size;
    cell.h = cell_size;



    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            cell.x = x + (i * cell_size);
            cell.y = y + (j * cell_size);

            SDL_RenderDrawRect(renderer, &cell);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 25);
    SDL_Rect outline;
    outline.x = x;
    outline.y = y;
    outline.w = cell_size * GRID_SIZE;
    outline.h = cell_size * GRID_SIZE;
    SDL_RenderDrawRect(renderer, &outline);



#else
#define m 333

    for(int i = 0; i < 15; i++) {




        //SDL_SetRenderDrawColor(renderer, 255 - i * m, 255 - i * m, 255 - i * m, 255 - i);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 255 - i * m, 255 - i);

        SDL_Rect outline;
        outline.x = x - i;
        outline.y = y - i;
        outline.w = cell_size * GRID_SIZE + i + i;
        outline.h = cell_size * GRID_SIZE + i + i;

        SDL_RenderDrawRect(renderer, &outline);
    }

    SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
    //SDL_Rect outline;
    outline.x = x;
    outline.y = y;
    outline.w = cell_size * GRID_SIZE;
    outline.h = cell_size * GRID_SIZE;
    SDL_RenderDrawRect(renderer, &outline);


#endif
    return;
}


void gen_apple()
{

    bool in_snake;

    do {
        in_snake = false;
        Apple.x = rand() % GRID_SIZE;    
        Apple.y = rand() % GRID_SIZE;    

        Snake *track = head;

        while(track != NULL) {
            if(track->x == Apple.x && track->y == Apple.y) {
                in_snake = true;
            }

            track = track->next;
        }
    }
    while(in_snake);


    Apple.score++;

    return;
}

void render_apple(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x00, 0x00, 255);

    int apple_size = win.gd / GRID_SIZE;

    SDL_Rect app;
    app.w = apple_size;
    app.h = apple_size;
    app.x = x + Apple.x * apple_size;
    app.y = y + Apple.y * apple_size;

    SDL_RenderFillRect(renderer, &app);

}

void detect_apple()
{
    if(head->x == Apple.x && head->y == Apple.y) {
        gen_apple();
        increase_snake();
    }

    return;
}

void detect_crash()
{
    if(head->x < 0 || head->x >= GRID_SIZE || head->y < 0 || head->y >= GRID_SIZE) {
        reset_snake();   
    }

    Snake *track = head;

    if(track->next != NULL) {
        track = track->next;
    }

    while(track != NULL) {
        if(track->x == head->x && track->y == head->y) {
            reset_snake();   
        }
        track = track->next;
    }


    return;
}

void turn_left()
{
    switch(head->dir) {
        case SNAKE_UP:
            head->dir = SNAKE_LEFT;
            break;
        case SNAKE_DOWN:
            head->dir = SNAKE_RIGHT;
            break;
        case SNAKE_LEFT:
            head->dir = SNAKE_DOWN;
            break;
        case SNAKE_RIGHT:
            head->dir = SNAKE_UP;
            break;
    }

    return;
}

void turn_right()
{
    switch(head->dir) {
        case SNAKE_UP:
            head->dir = SNAKE_RIGHT;
            break;
        case SNAKE_DOWN:
            head->dir = SNAKE_LEFT;
            break;
        case SNAKE_LEFT:
            head->dir = SNAKE_UP;
            break;
        case SNAKE_RIGHT:
            head->dir = SNAKE_DOWN;
            break;
    }

    return;
}

enum {
    TRY_FORWARD,
    TRY_LEFT,
    TRY_RIGHT,
};

int future_state(int try, int x, int y, int dir)
{
    int reward = 0;

    int try_x = x;
    int try_y = y;


    switch(dir) {
        case SNAKE_UP:
            switch(try) {
                case TRY_FORWARD:
                    try_y--;
                    break;
                case TRY_LEFT:
                    try_x--;
                    break;
                case TRY_RIGHT:
                    try_x++;
                    break;
            }
            break;
        case SNAKE_DOWN:
            switch(try) {
                case TRY_FORWARD:
                    try_y++;
                    break;
                case TRY_LEFT:
                    try_x++;
                    break;
                case TRY_RIGHT:
                    try_x--;
                    break;
            }
            break;
        case SNAKE_LEFT:
            switch(try) {
                case TRY_FORWARD:
                    try_x--;
                    break;
                case TRY_LEFT:
                    try_y++;
                    break;
                case TRY_RIGHT:
                    try_y--;
                    break;
            }
            break;
        case SNAKE_RIGHT:
            switch(try) {
                case TRY_FORWARD:
                    try_x++;
                    break;
                case TRY_LEFT:
                    try_y--;
                    break;
                case TRY_RIGHT:
                    try_y++;
                    break;
            }
            break;
    }

    //DETECT WALL CRASH
    if(Data.reward_wall_crash == true)  {
        if(try_x < 0 || try_x > GRID_SIZE - 1) {
            reward += -100;
        }
        if(try_y < 0 || try_y > GRID_SIZE - 1) {
            reward += -100;
        }

    }

    //DETECT APPLE
    if(Data.reward_apple == true) {

        if(try_x == Apple.x && try_y == Apple.y) {
            reward += 100;
        }
    }

    //MOVE TOWARDS APPLE
    if(Data.reward_move_to_apple == true) {
        int diff_x = abs(head->x - Apple.x);
        int diff_y = abs(head->y - Apple.y);
        int try_diff_x = abs(try_x - Apple.x);
        int try_diff_y = abs(try_y - Apple.y);
        
        if(try_diff_x < diff_x) {
            reward += 5;
        }
        if(try_diff_y < diff_y) {
            reward += 5;
        }
    }


    //DETECT TAIL
    if(Data.reward_tail_crash == true) {

        Snake *track = head;

        if(track->next != NULL) {
            track = track->next;
        }
        while(track != NULL) {
            if(try_x == track->x && try_y == track->y) {
                reward += -100;
            }

            track = track->next;
        }
    }


    //DETECT GHOST TAIL
    
    Snake *track = g_head;
    if(track->next != NULL) {
        track = track->next;
    }
    while(track != NULL) {
        if(try_x == track->x && try_y == track->y) {
            reward += -50;
        }
        track = track->next;
    }

    return reward;
}

int change_dir(int try, int dir)
{
    switch(dir) {
        case SNAKE_UP:
            switch(try) {
                case TRY_FORWARD:
                    dir = SNAKE_UP;
                    break;
                case TRY_LEFT:
                    dir = SNAKE_LEFT;
                    break;
                case TRY_RIGHT:
                    dir = SNAKE_RIGHT;
                    break;
            }
            break;
        case SNAKE_DOWN:
            switch(try) {
                case TRY_FORWARD:
                    dir = SNAKE_DOWN;
                    break;
                case TRY_LEFT:
                    dir = SNAKE_RIGHT;
                    break;
                case TRY_RIGHT:
                    dir = SNAKE_LEFT;
                    break;
            }
            break;
        case SNAKE_LEFT:
            switch(try) {
                case TRY_FORWARD:
                    dir = SNAKE_LEFT;
                    break;
                case TRY_LEFT:
                    dir = SNAKE_DOWN;
                    break;
                case TRY_RIGHT:
                    dir = SNAKE_UP;
                    break;
            }
            break;
        case SNAKE_RIGHT:
            switch(try) {
                case TRY_FORWARD:
                    dir = SNAKE_RIGHT;
                    break;
                case TRY_LEFT:
                    dir = SNAKE_UP;
                    break;
                case TRY_RIGHT:
                    dir = SNAKE_DOWN;
                    break;
            }
            break;
    }
    return dir;
}

int state(int try, SDL_Renderer *renderer, int x, int y)
{
    int reward = 0;

    int try_x = head->x;
    int try_y = head->y;


    switch(head->dir) {
        case SNAKE_UP:
            switch(try) {
                case TRY_FORWARD:
                    try_y--;
                    break;
                case TRY_LEFT:
                    try_x--;
                    break;
                case TRY_RIGHT:
                    try_x++;
                    break;
            }
            break;
        case SNAKE_DOWN:
            switch(try) {
                case TRY_FORWARD:
                    try_y++;
                    break;
                case TRY_LEFT:
                    try_x++;
                    break;
                case TRY_RIGHT:
                    try_x--;
                    break;
            }
            break;
        case SNAKE_LEFT:
            switch(try) {
                case TRY_FORWARD:
                    try_x--;
                    break;
                case TRY_LEFT:
                    try_y++;
                    break;
                case TRY_RIGHT:
                    try_y--;
                    break;
            }
            break;
        case SNAKE_RIGHT:
            switch(try) {
                case TRY_FORWARD:
                    try_x++;
                    break;
                case TRY_LEFT:
                    try_y--;
                    break;
                case TRY_RIGHT:
                    try_y++;
                    break;
            }
            break;
    }

    //DETECT WALL CRASH
    if(Data.reward_wall_crash == true)  {
        if(try_x < 0 || try_x > GRID_SIZE - 1) {
            reward += -100;
        }
        if(try_y < 0 || try_y > GRID_SIZE - 1) {
            reward += -100;
        }

    }
    //DETECT APPLE
    if(Data.reward_apple == true) {

        if(try_x == Apple.x && try_y == Apple.y) {
            reward += 100;
        }
    }

    //MOVE TOWARDS APPLE
    if(Data.reward_move_to_apple == true) {
        int diff_x = abs(head->x - Apple.x);
        int diff_y = abs(head->y - Apple.y);
        int try_diff_x = abs(try_x - Apple.x);
        int try_diff_y = abs(try_y - Apple.y);
        
        if(try_diff_x < diff_x) {
            reward += 5;
        }
        if(try_diff_y < diff_y) {
            reward += 5;
        }
    }


    //DETECT TAIL
    if(Data.reward_tail_crash == true) {

        Snake *track = head;

        if(track->next != NULL) {
            track = track->next;
        }
        while(track != NULL) {
            if(try_x == track->x && try_y == track->y) {
                reward += -100;
            }

            track = track->next;
        }
    }

    //STAY AWAY FROM WALLS
    if(head->x > 3 || head->x < GRID_SIZE - 4) {
        reward += -40;
    }
    if(head->y > 3 || head->y < GRID_SIZE - 4) {
        reward += -40;
    }



    //DETECT FUTURE

    int dir = change_dir(try, head->dir);





    init_ghost(try_x, try_y, dir);

    bool still_alive = true;
    int epochs = 0;
    while(epochs < (Apple.score + 4) && still_alive && epochs < (GRID_SIZE)) {
        epochs++;
        
        int new_f = future_state(TRY_FORWARD, try_x, try_y, dir);
        int new_l = future_state(TRY_LEFT, try_x, try_y, dir);
        int new_r = future_state(TRY_RIGHT, try_x, try_y, dir);

        if(new_f >= new_l && new_f >= new_r) {
            //CONTINUE FOWARD
            switch(dir) {
                case SNAKE_UP:
                    try_y--;
                    break;
                case SNAKE_DOWN:
                    try_y++;
                    break;
                case SNAKE_LEFT:
                    try_x--;
                    break;
                case SNAKE_RIGHT:
                    try_x++;
                    break;
            }
        }
        else {
            if(new_l == new_r) {
                int r = rand() % 2;
                switch(r) {
                    case 0:
                        switch(dir) {
                            case SNAKE_UP:
                                dir = SNAKE_LEFT;
                                try_x--;
                                break;
                            case SNAKE_DOWN:
                                dir = SNAKE_RIGHT;
                                try_x++;
                                break;
                            case SNAKE_LEFT:
                                dir = SNAKE_DOWN;
                                try_y++;
                                break;
                            case SNAKE_RIGHT:
                                dir = SNAKE_UP;
                                try_y--;
                                break;
                        }
                        break;
                    case 1:
                        switch(dir) {
                            case SNAKE_UP:
                                dir = SNAKE_RIGHT;
                                try_x++;
                                break;
                            case SNAKE_DOWN:
                                dir = SNAKE_LEFT;
                                try_x--;
                                break;
                            case SNAKE_LEFT:
                                dir = SNAKE_UP;
                                try_y--;
                                break;
                            case SNAKE_RIGHT:
                                dir = SNAKE_DOWN;
                                try_y++;
                                break;
                        }
                        break;
                }
            }
            else if(new_l > new_r) {
                //turn_left();
                switch(dir) {
                    case SNAKE_UP:
                        dir = SNAKE_LEFT;
                        try_x--;
                        break;
                    case SNAKE_DOWN:
                        dir = SNAKE_RIGHT;
                        try_x++;
                        break;
                    case SNAKE_LEFT:
                        dir = SNAKE_DOWN;
                        try_y++;
                        break;
                    case SNAKE_RIGHT:
                        dir = SNAKE_UP;
                        try_y--;
                        break;
                }
            }
            else {
                //turn_right();
                switch(dir) {
                    case SNAKE_UP:
                        dir = SNAKE_RIGHT;
                        try_x++;
                        break;
                    case SNAKE_DOWN:
                        dir = SNAKE_LEFT;
                        try_x--;
                        break;
                    case SNAKE_LEFT:
                        dir = SNAKE_UP;
                        try_y--;
                        break;
                    case SNAKE_RIGHT:
                        dir = SNAKE_DOWN;
                        try_y++;
                        break;
                }
            }



        }

        Snake *track = head;
        while(track != NULL) {

            Snake *track2 = g_head;
            while(track2 != NULL) {
                if(track2->x == track->x && track2->y == track->y) {
                    still_alive = false;
                }
                track2 = track2->next;
            }


            /*
            if(try_x == track->x && try_y == track->y) {
                still_alive = false;
            }
            */
            track = track->next;
        }

/*
        track = g_head;
        while(track != NULL) {
            if(try_x == track->x && try_y == track->y) {
                still_alive = false;
            }
            track = track->next;
        }
*/


        increase_ghost(try_x, try_y);

    }









    if(!still_alive) {
        reward += - 6;
        //printf("futureDead");
    }


    render_ghost(renderer, x, y);

    reset_ghost();

    return reward;
}

void ai(SDL_Renderer *renderer, int x, int y)
{
    int try_f = state(TRY_FORWARD, renderer, x, y);
    int try_l = state(TRY_LEFT, renderer, x, y);
    int try_r = state(TRY_RIGHT, renderer, x, y);

    if(try_f >= try_l && try_f >= try_r) {
        //CONTINUE FOWARD
    }
    else {
        if(try_l > try_r) {
            turn_left();
        }
        else {
            turn_right();
        }

    }

    Data.f = try_f;
    Data.l = try_l;
    Data.r = try_r;

    return;
}

void render_score(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 4;

    SDL_Rect cell;
    cell.w = cell_size - 1;
    cell.h = cell_size - 1;

    char buff[10];
    snprintf(buff, sizeof(buff), "%4d", Apple.score);

    for(int k = 0; k < 4; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {

                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255);

                    cell.x = x + cell_size * i + (cell_size * 9 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);

                }
            }
        }
    }

    return;
}

void render_top_score(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255);

    int cell_size = 4;

    SDL_Rect cell;
    cell.w = cell_size - 1;
    cell.h = cell_size - 1;

    char buff[10];
    snprintf(buff, sizeof(buff), "%4d", Apple.top_score);

    for(int k = 0; k < 4; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 9 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_title(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 4;

    SDL_Rect cell;
    cell.w = cell_size - 0.5;
    cell.h = cell_size - 0.5;

    char buff[20] = {'S','N','A','K','E'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}
void render_sub_title(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0.5;
    cell.h = cell_size - 0.5;

    char buff[20] = {'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_spec(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'C',':','S','D','L','2',':','Q','-','L','E','A','R','N','I','N','G',' '};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}


void render_arrow(SDL_Renderer *renderer, int x, int y)
{





    int cell_size = 31;

    int off = 14;

    SDL_Rect cell;
    cell.w = cell_size - off;
    cell.h = cell_size - off;
/*
    char buff[10];
    snprintf(buff, sizeof(buff), "%4d", head->dir);
*/


    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 25);
    SDL_Rect out;
    out.w = cell_size * 9;
    out.h = cell_size * 9;
    out.x = x;
    out.y = y;
    SDL_RenderDrawRect(renderer, &out);

    char buff;
    switch(head->dir) {
        case SNAKE_UP:
            buff = 38;
            break;
        case SNAKE_DOWN:
            buff = 40;
            break;
        case SNAKE_LEFT:
            buff = 37;
            break;
        case SNAKE_RIGHT:
            buff = 39;
            break;
    }

    for(int k = 0; k < 1; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff][j][i]) {

                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 25);




                    cell.x = x + cell_size * i + (cell_size * 9 * k) + off / 2;
                    cell.y = y + cell_size * j + off / 2;
                    SDL_RenderFillRect(renderer, &cell);

                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255);

                    SDL_RenderDrawRect(renderer, &cell);


                }
                else {

                    SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);

                    cell.x = x + cell_size * i + (cell_size * 9 * k) + off/2;
                    cell.y = y + cell_size * j + off / 2;
                    SDL_RenderFillRect(renderer, &cell);

                    SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 155);
                    SDL_RenderDrawRect(renderer, &cell);

                }
            }
        }
    }

    return;
}



void render_score_label(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'S','C','O','R','E',':',};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_top_score_label(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'T','O','P',':',};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_forward_label(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'F','O','R','W','A','R','D',':'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}


void render_forward_val(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 1;
    cell.h = cell_size - 1;

    char buff[10];
    snprintf(buff, sizeof(buff), "%4d", Data.f);

    for(int k = 0; k < 4; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {

                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255);

                    cell.x = x + cell_size * i + (cell_size * 9 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);

                }
            }
        }
    }

    return;
}

void render_left_label(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'L','E','F','T'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}


void render_left_val(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 1;
    cell.h = cell_size - 1;

    char buff[10];
    snprintf(buff, sizeof(buff), "%4d", Data.l);

    for(int k = 0; k < 4; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {

                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255);

                    cell.x = x + cell_size * i + (cell_size * 9 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);

                }
            }
        }
    }

    return;
}

void render_right_label(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'R','I','G','H','T'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}


void render_right_val(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 1;
    cell.h = cell_size - 1;

    char buff[10];
    snprintf(buff, sizeof(buff), "%4d", Data.r);

    for(int k = 0; k < 4; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {

                    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255);

                    cell.x = x + cell_size * i + (cell_size * 9 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);

                }
            }
        }
    }

    return;
}


void render_toggle_pause(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'P',' ',':',' ','P','A','U','S','E'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_toggle_reset(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'R',' ',':',' ','R','E','S','E','T'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_toggle_walls(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'W',' ',':',' ','W','A','L','L','S'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_toggle_apple(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'A',' ',':',' ','A','P','P','L','E'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_toggle_move(SDL_Renderer *renderer, int x, int y)
{

    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);

    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'M',' ',':',' ','M','O','V','E'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_toggle_tail(SDL_Renderer *renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);


    int cell_size = 3;

    SDL_Rect cell;
    cell.w = cell_size - 0;
    cell.h = cell_size - 0.5;

    char buff[20] = {'T',' ',':',' ','T','A','I','L'};

    //snprintf(buff, sizeof(buff), "%s", "Snake");

    for(int k = 0; k < 20; k++) {
        for(int i = 0; i < 9; i++) {
            for(int j = 0; j < 9; j++) {
                if(glyphs[buff[k]][j][i]) {
                    cell.x = x + cell_size * i + (cell_size * 7 * k);
                    cell.y = y + cell_size * j;
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }
    }

    return;
}

void render_toggle_walls_check(SDL_Renderer *renderer, int x, int y )
{


    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);
    SDL_Rect check;
    check.w = 20;
    check.h = 20;
    check.x = x;
    check.y = y;


    if(Data.reward_wall_crash == true) {
        SDL_RenderFillRect(renderer, &check);
    }
    else {
        SDL_RenderDrawRect(renderer, &check);
    }

    return;
}

void render_toggle_apple_check(SDL_Renderer *renderer, int x, int y )
{


    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);
    SDL_Rect check;
    check.w = 20;
    check.h = 20;
    check.x = x;
    check.y = y;


    if(Data.reward_apple == true) {
        SDL_RenderFillRect(renderer, &check);
    }
    else {
        SDL_RenderDrawRect(renderer, &check);
    }

    return;
}

void render_toggle_move_check(SDL_Renderer *renderer, int x, int y )
{


    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);
    SDL_Rect check;
    check.w = 20;
    check.h = 20;
    check.x = x;
    check.y = y;


    if(Data.reward_move_to_apple == true) {
        SDL_RenderFillRect(renderer, &check);
    }
    else {
        SDL_RenderDrawRect(renderer, &check);
    }

    return;
}

void render_toggle_tail_check(SDL_Renderer *renderer, int x, int y )
{


    SDL_SetRenderDrawColor(renderer, 0x55, 0x99, 0xFF, 255);
    SDL_Rect check;
    check.w = 20;
    check.h = 20;
    check.x = x;
    check.y = y;


    if(Data.reward_tail_crash == true) {
        SDL_RenderFillRect(renderer, &check);
    }
    else {
        SDL_RenderDrawRect(renderer, &check);
    }

    return;
}

int logo_a = 255;
int logo_dir = 0;
void render_logo(SDL_Renderer *renderer, int x, int y)
{
    if(logo_dir == 0) {
        logo_a -= 1;
        if(logo_a < 150) {
            logo_dir = 1;
        }
    }
    if(logo_dir == 1) {
        logo_a += 1;
        if(logo_a > 250) {
            logo_dir = 0;
        }
    }


    SDL_Color c;
    c.r = 0x55;
    c.g = 0x99;
    c.b = 0xFF;
    c.a = logo_a;

    int r = 40;
    SDL_RenderFillCircle(renderer, x, y, r, c);

    c.r = 0x09;
    c.g = 0x09;
    c.b = 0x09;
    c.a = 255;

    r = 30;
    SDL_RenderFillCircle(renderer, x, y, r, c);

    SDL_Rect rect;
    rect.w = 80;
    rect.h = 80;
    rect.x = x;
    rect.y = y - 40;
    SDL_RenderFillRect(renderer, &rect);


    c.r = 0x55;
    c.g = 0x99;
    c.b = 0xFF;
    c.a = logo_a;

    r = 22;
    SDL_RenderFillCircle(renderer, x, y, r, c);




    SDL_RenderFillCircle(renderer, x, y, r, c);

    rect.w = 20;
    rect.h = 10;
    rect.x = x + 20;
    rect.y = y - 5;

    SDL_RenderFillRect(renderer, &rect);

    rect.w = 20;
    rect.h = 10;
    rect.x = x - 59;
    rect.y = y - 5;
    SDL_RenderFillRect(renderer, &rect);

    return;
}

int main()
{
	
    srand(time(0));

    init_snake();
    increase_snake();
    increase_snake();
    increase_snake();

    gen_apple();
    Apple.score = 0;


    Data.paused = false;
    Data.reward_wall_crash = true;
    Data.reward_apple = true;
    Data.reward_move_to_apple = true;
    Data.reward_tail_crash = true;




    SDL_Window *window;
    SDL_Renderer *renderer;


    if(SDL_INIT_VIDEO < 0) {
        fprintf(stderr, "ERROR: SDL_INIT_VIDEO");
    }

    window = SDL_CreateWindow(
        "Snake",
        WINDOW_X,
        WINDOW_Y,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    if(!window) {
        fprintf(stderr, "ERROR: !window");
    }




    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(!renderer) {
        fprintf(stderr, "!renderer");
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_GetRendererOutputSize(renderer, &win.w, &win.h);




    int grid_x = (win.w / 2) - (win.gd / 2);
    int grid_y = (win.h / 2) - (win.gd / 2);

    int flash = 0;
    bool quit = false;
    SDL_Event event;




    while(!quit) {
        while(SDL_PollEvent(&event)) {

			SDL_GetRendererOutputSize(renderer, &win.w, &win.h);
			grid_x = (win.w / 2) - (win.gd / 2);
			grid_y = (win.h / 2) - (win.gd / 2);
			win.gd = win.h - 400;

			if(win.w > win.h) {
				win.gd = win.h - 300;
			}
			else {
				win.gd = win.w - 300;
			}


            switch(event.type) {
    		case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYUP:
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                        case SDLK_UP:
                            head->dir = SNAKE_UP;
                            break;
                        case SDLK_DOWN:
                            head->dir = SNAKE_DOWN;
                            break;
                        case SDLK_LEFT:
                            head->dir = SNAKE_LEFT;
                            break;
                        case SDLK_RIGHT:
                            head->dir = SNAKE_RIGHT;
                            break;
                        case SDLK_p:
                            if(Data.paused == false) {
                                Data.paused = true;
                                break;
                            }
                            else {
                                Data.paused = false;
                            }
                            break;
                        case SDLK_r:
                            reset_snake();
                            break;
                        case SDLK_w:
                            if(Data.reward_wall_crash == false) {
                                Data.reward_wall_crash = true;

                            }
                            else {
                                Data.reward_wall_crash = false;
                            }

                            break;
                        case SDLK_m:
                            if(Data.reward_move_to_apple == false) {
                                Data.reward_move_to_apple = true;

                            }
                            else {
                                Data.reward_move_to_apple = false;
                            }

                            break;
                        case SDLK_t:
                            if(Data.reward_tail_crash == false) {
                                Data.reward_tail_crash = true;

                            }
                            else {
                                Data.reward_tail_crash = false;
                            }

                            break;
                        case SDLK_a:
                            if(Data.reward_apple == false) {
                                Data.reward_apple = true;

                            }
                            else {
                                Data.reward_apple = false;
                            }

                            break;
                    }
                    break;
            }
        }

        SDL_RenderClear(renderer);
        //RENDER LOOP START

        if(Data.paused == false) {
        move_snake();
        detect_apple();
        detect_crash();
        }



        render_grid(renderer, grid_x, grid_y);
        render_snake(renderer, grid_x, grid_y);


        if(Apple.score % 10 == 0 && Apple.score != 0 ) {
            flash = 10;
        }
        if(flash > 0) {
            flash_snake(renderer, grid_x, grid_y);
            flash--;
        }

        render_apple(renderer, grid_x, grid_y);


        render_score(renderer, win.w - 500, 25);
        render_top_score(renderer, win.w - 500, 85);
        render_title(renderer,20, 20);
        render_sub_title(renderer,20, 60);
        render_spec(renderer,20, 90);
        render_arrow(renderer,20, 290);
        render_score_label(renderer, win.w - 600, 30 );
        render_top_score_label(renderer, win.w - 560, 90 );

        //VALS
        render_forward_val(renderer, 200, 800 );
        render_forward_label(renderer, 20, 800 );

        render_left_val(renderer, 200, 900 );
        render_left_label(renderer, 20, 900 );

        render_right_val(renderer, 200, 1000 );
        render_right_label(renderer, 20, 1000 );

        //TOGGLES
        render_toggle_pause(renderer, win.w - 320, 300 );
        render_toggle_reset(renderer, win.w - 320, 350 );


        render_toggle_walls(renderer, win.w - 320, 750 );
        render_toggle_walls_check(renderer, win.w - 50, 750 );

        render_toggle_apple(renderer, win.w - 320, 800 );
        render_toggle_apple_check(renderer, win.w - 50, 800 );

        render_toggle_move(renderer, win.w - 320, 850 );
        render_toggle_move_check(renderer, win.w - 50, 850 );

        render_toggle_tail(renderer, win.w - 320, 900 );
        render_toggle_tail_check(renderer, win.w - 50, 900 );


        render_logo(renderer, win.w / 2  , win.h - 80);


        ai(renderer, grid_x, grid_y);

        //RENDER LOOP END
        //SDL_SetRenderDrawColor(renderer, 0xe5, 0xe5,0xe5, 255);
        SDL_SetRenderDrawColor(renderer, 0x09, 0x09,0x09, 255);
        SDL_RenderPresent(renderer);

        
        SDL_Delay(DELAY);

    }








    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}
