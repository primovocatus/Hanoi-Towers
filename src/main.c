#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COL_PER_CELL 2

int disk_count = 6;

typedef struct disk {
    int size, color;
};

struct disk* create_disk(int size, int color) {
    struct disk* Disk = malloc(sizeof(struct disk));

    Disk->size = size;
    Disk->color = color;

    return Disk;
}

typedef struct tower {
    WINDOW* win;

    int x, y;

    int size;
    struct disk* stack[20];
};

struct tower* init(int y, int x) {
    struct tower* Tower = malloc(sizeof(struct tower));
    Tower->win = newwin(disk_count + 2, COL_PER_CELL * (2 * disk_count + 1), y, x);

    Tower->y = y + disk_count + 1, Tower->x = x;

    Tower->size = 0;
    memset(Tower->stack, 0, 20 * sizeof(struct disk*));
    return Tower;
}

void add_disk(struct tower* Tower, struct disk* Disk) {
    Tower->stack[Tower->size] = Disk;

    wattrset(Tower->win, COLOR_PAIR(Disk->color));

    wattron(Tower->win, A_REVERSE);
    for (int i = 0; i < COL_PER_CELL * (disk_count - Disk->size); ++i) {
        mvwprintw(Tower->win, Tower->y - Tower->size, i, " ");
    }

    wattroff(Tower->win, A_REVERSE);
    for (int i = 0; i < COL_PER_CELL * (2 * Disk->size + 1); ++i) {
        mvwprintw(Tower->win, Tower->y - Tower->size, (COL_PER_CELL * (disk_count - Disk->size)) + i, " ");
    }
    Tower->size++;

    wrefresh(Tower->win);
}

void del_disk(struct tower* Tower) {
    Tower->size--;
    Tower->stack[Tower->size] = 0;

    wattrset(Tower->win, COLOR_PAIR(0));
    for (int i = 0; i < COL_PER_CELL * (2 * disk_count + 1); ++i) {
        mvwprintw(Tower->win, Tower->y - Tower->size, i, " ");
    }

    wrefresh(Tower->win);
}

struct tower* Tower[3];
int odd[3], even[3];

int position[20];

int cur_state = 1;
int gray_code = 0;

void solve() {
    while (Tower[1]->size != disk_count) {
        gray_code++;
        int current_gray = ((gray_code - 1) ^ ((gray_code - 1) >> 1)) ^ (gray_code ^ (gray_code >> 1));
        int current_disk = 0;

        while (!(current_gray & 1)) {
            current_disk++, current_gray >>= 1;
        }

        struct disk* current = Tower[position[current_disk]]->stack[Tower[position[current_disk]]->size - 1];

        if (current_disk == 0) {
            add_disk(Tower[(disk_count & 1 ? odd[cur_state] : even[cur_state])], current);
            del_disk(Tower[position[current_disk]]);

            position[current_disk] = (disk_count & 1 ? odd[cur_state] : even[cur_state]);
            cur_state = (cur_state + 1) % 3;
        } else {
            for (int i = 0; i < 3; ++i) {
                if (position[current_disk] == i) continue;

                if (Tower[i]->size == 0 || Tower[i]->stack[Tower[i]->size - 1]->size > current_disk + 1) {
                    add_disk(Tower[i], current);
                    del_disk(Tower[position[current_disk]]);

                    position[current_disk] = i;
                    break;
                }
            }
        }

        usleep(500000);
    }
}

int main() {
    initscr();

    noecho();
    cbreak();
    curs_set(FALSE);

    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();

        init_pair(1, COLOR_BLACK, COLOR_RED);
        init_pair(2, COLOR_BLACK, COLOR_BLUE);
        init_pair(3, COLOR_BLACK, COLOR_GREEN);
        init_pair(4, COLOR_BLACK, COLOR_YELLOW);
    }

    int MAX_X, MAX_Y;
    getmaxyx(stdscr, MAX_Y, MAX_X);

    Tower[0] = init(0, 2);
    Tower[1] = init(0, COL_PER_CELL * (2 * disk_count + 2) + 2);
    Tower[2] = init(0, 2 * COL_PER_CELL * (2 * disk_count + 2) + 2);

    int current_color = 4;
    for (int i = disk_count; i >= 1; --i) {
        add_disk(Tower[0], create_disk(i, current_color));
        position[i - 1] = 0;

        current_color--;
        if (current_color == 0) current_color = 4;
    }

    odd[0] = 0, odd[1] = 1, odd[2] = 2;
    even[0] = 0, even[1] = 2, even[2] = 1;

    solve();

    return 0;
}