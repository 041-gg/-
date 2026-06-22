#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINES 1000
#define MAX_LINE_LEN 1024
#define MAX_STACK_SIZE 100
#define MAX_WORDS 5000    
#define MAX_KEYWORDS 20  
#define COLOR_RED "\033[31m"  
#define COLOR_RESET "\033[0m" 
typedef struct LineNode {
    int line_no;                
    char content[MAX_LINE_LEN]; 
    struct LineNode *prev;      
    struct LineNode *next;      
} LineNode;
typedef struct {
    LineNode *head;    
    LineNode *tail;    
    int line_count;    
} TextGraph;

typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;
TextGraph undo_stack[MAX_STACK_SIZE];
int stack_top = -1;
LineNode* create_line_node(int num, const char* str) {
    LineNode* node = (LineNode*)malloc(sizeof(LineNode));
    node->line_no = num;
    strcpy(node->content, str);
    node->prev = NULL;
    node->next = NULL;
    return node;
}
TextGraph* create_text_graph() {
    TextGraph* g = (TextGraph*)malloc(sizeof(TextGraph));
    g->head = NULL;
    g->tail = NULL;
    g->line_count = 0;
    return g;
}
void graph_append(TextGraph* g, const char* line_str) {
    if (!g) return;
    LineNode* new_node = create_line_node(g->line_count + 1, line_str);
    if (g->line_count == 0) {
        g->head = new_node;
        g->tail = new_node;
    } else {
        g->tail->next = new_node;
        new_node->prev = g->tail;
        g->tail = new_node;
    }
    g->line_count++;
}

LineNode* find_node_by_lineno(TextGraph* g, int target_no) {
    if (!g || g->line_count == 0) return NULL;
    LineNode* cur = g->head;
    while (cur) {
        if (cur->line_no == target_no) return cur;
        cur = cur->next;
    }
    return NULL;
}
int graph_delete_node(TextGraph* g, int line_num) {
    LineNode* del = find_node_by_lineno(g, line_num);
    if (!del) return 0;
    if (del->prev == NULL) {
        g->head = del->next;
        if (g->head) g->head->prev = NULL;
    }
    else if (del->next == NULL) {
        g->tail = del->prev;
        g->tail->next = NULL;
    }
    else {
        del->prev->next = del->next;
        del->next->prev = del->prev;
    }
    free(del);
    g->line_count--;
    return 1;
}
TextGraph copy_graph(const TextGraph* src) {
    TextGraph dst = {NULL, NULL, 0};
    if (!src || src->line_count == 0) return dst;
    LineNode* cur = src->head;
    while (cur) {
        graph_append(&dst, cur->content);
        cur = cur->next;
    }
    return dst;
}
void free_graph(TextGraph* g) {
    if (!g) return;
    LineNode* cur = g->head;
    while (cur) {
        LineNode* tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    free(g);
}
TextGraph* load_file(const char* file_path) {
    TextGraph* g = create_text_graph();
    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        printf("文件打开失败\n");
        free_graph(g);
        return NULL;
    }
    char buf[MAX_LINE_LEN];
    while (g->line_count < MAX_LINES && fgets(buf, MAX_LINE_LEN, fp)) {
        char* nl = strchr(buf, '\n');
        if (nl) *nl = '\0';
        graph_append(g, buf);
    }
    fclose(fp);
    return g;
}
void display_text(const TextGraph* g) {
    if (!g || g->line_count == 0) {
        printf("[文本为空]\n");
        return;
    }
    printf("\n===== 文本内容 =====\n");
    LineNode* cur = g->head;
    while (cur) {
        printf("%d: %s\n", cur->line_no, cur->content);
        cur = cur->next;
    }
    printf("====================\n\n");
}
int save_state(TextGraph* g) {
    if (stack_top >= MAX_STACK_SIZE - 1) {
        printf("撤销栈已满\n");
        return 0;
    }
    stack_top++;
    undo_stack[stack_top] = copy_graph(g);
    return 1;
}

int undo_delete(TextGraph* g) {
    if (stack_top < 0) {
        printf("无可撤销操作\n");
        return 0;
    }
    free_graph(g);
    *g = copy_graph(&undo_stack[stack_top]);
    stack_top--;
    printf("已撤销上一次删除\n");
    return 1;
}
int delete_line(TextGraph* g, int line_num) {
    if (!g || line_num < 1 || line_num > g->line_count) {
        printf("无效行号\n");
        return 0;
    }
    if (!save_state(g)) return 0;
    graph_delete_node(g, line_num);
    printf("已删除第 %d 行\n", line_num);
    return 1;
}
int count_total_lines(const TextGraph* g) {
    return g ? g->line_count : 0;
}

int count_total_chars(const TextGraph* g) {
    if (!g) return 0;
    int total = 0;
    LineNode* cur = g->head;
    while (cur) {
        total += strlen(cur->content);
        cur = cur->next;
    }
    return total;
}
void find_text(const TextGraph* g, const char* keyword) {
    if (!g || !keyword || !*keyword) {
        printf("查询参数无效\n");
        return;
    }
    int found = 0;
    int key_len = strlen(keyword);
    printf("\n===== 查找'%s' =====\n", keyword);
    LineNode* cur = g->head;
    while (cur) {
        const char* s = cur->content;
        int len = strlen(s);
        for (int j = 0; j <= len - key_len; j++) {
            int match = 1;
            for (int k = 0; k < key_len; k++) {
                if (s[j + k] != keyword[k]) {
                    match = 0; break;
                }
            }
            if (match) {
                printf("在 %d 行 第 %d 列\n", cur->line_no, j + 1);
                found = 1;
            }
        }
        cur = cur->next;
    }
    if (!found) printf("未找到\n");
    printf("====================\n\n");
}
void to_lower(char* s) {
    for (; *s; s++) *s = tolower(*s);
}

int is_valid_char(int c) {
    return isalpha(c);
}

int get_word(const char** p, char* word) {
    const char* s = *p;
    while (*s && !is_valid_char(*s)) s++;
    if (!*s) return 0;
    int i = 0;
    while (*s && is_valid_char(*s) && i < MAX_WORD_LEN - 1) {
        word[i++] = *s++;
    }
    word[i] = '\0';
    *p = s;
    return 1;
}

int count_word_frequency(const TextGraph* g, WordFreq* freq_list) {
    if (!g) return 0;
    int word_num = 0;
    memset(freq_list, 0, sizeof(WordFreq) * MAX_WORDS);
    LineNode* cur = g->head;
    while (cur) {
        const char* p = cur->content;
        char word[MAX_WORD_LEN];
        while (get_word(&p, word)) {
            to_lower(word);
            int found = 0;
            for (int j = 0; j < word_num; j++) {
                if (strcmp(freq_list[j].word, word) == 0) {
                    freq_list[j].count++;
                    found = 1; break;
                }
            }
            if (!found && word_num < MAX_WORDS) {
                strcpy(freq_list[word_num].word, word);
                freq_list[word_num].count = 1;
                word_num++;
            }
        }
        cur = cur->next;
    }
    return word_num;
}

int cmp_freq(const void* a, const void* b) {
    WordFreq* w1 = (WordFreq*)a;
    WordFreq* w2 = (WordFreq*)b;
    return w2->count - w1->count;
}

void sort_frequency(WordFreq* list, int n) {
    qsort(list, n, sizeof(WordFreq), cmp_freq);
}

void show_word_frequency(const TextGraph* g) {
    WordFreq freq_list[MAX_WORDS];
    int n = count_word_frequency(g, freq_list);
    if (n == 0) {
        printf("无单词可统计\n");
        return;
    }
    sort_frequency(freq_list, n);
    printf("\n===== 词频统计（降序）=====\n");
    for (int i = 0; i < n; i++) {
        printf("%-15s %d\n", freq_list[i].word, freq_list[i].count);
    }
    printf("==========================\n\n");
}
int split_keywords(const char* input, char keywords[MAX_KEYWORDS][MAX_WORD_LEN]) {
    if (!input || !*input) return 0;
    int count = 0;
    const char* p = input;
    while (*p && count < MAX_KEYWORDS) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        int i = 0;
        while (*p && !isspace((unsigned char)*p) && i < MAX_WORD_LEN - 1) {
            keywords[count][i++] = *p++;
        }
        keywords[count][i] = '\0';
        count++;
    }
    return count;
}

void highlight_keywords(const TextGraph* g, const char* input_keywords) {
    if (!g || !input_keywords || !*input_keywords) {
        printf("关键词参数无效\n");
        return;
    }
    char keywords[MAX_KEYWORDS][MAX_WORD_LEN];
    int keyword_count = split_keywords(input_keywords, keywords);
    if (keyword_count == 0) {
        printf("未识别到有效关键词\n");
        return;
    }
    printf("\n===== 多关键词高亮展示 =====\n");
    LineNode* cur = g->head;
    while (cur) {
        const char* line = cur->content;
        int line_len = strlen(line);
        char highlighted_line[MAX_LINE_LEN * 2] = {0};
        int pos = 0;
        for (int i = 0; i < line_len; ) {
            int matched = 0;
            for (int k = 0; k < keyword_count; k++) {
                int key_len = strlen(keywords[k]);
                if (i + key_len <= line_len && strncmp(&line[i], keywords[k], key_len) == 0) {
                    strcpy(&highlighted_line[pos], COLOR_RED);
                    pos += strlen(COLOR_RED);
                    strncpy(&highlighted_line[pos], &line[i], key_len);
                    pos += key_len;
                    strcpy(&highlighted_line[pos], COLOR_RESET);
                    pos += strlen(COLOR_RESET);
                    i += key_len;
                    matched = 1;
                    break;
                }
            }
            if (!matched) {
                highlighted_line[pos++] = line[i++];
            }
        }
        printf("%d: %s\n", cur->line_no, highlighted_line);
        cur = cur->next;
    }
    printf("===========================\n\n");
}
int main() {
    TextGraph* text = load_file("test.txt");
    if (!text) return 1;
    display_text(text);
    printf("总行数：%d\n", count_total_lines(text));
    printf("总字符数：%d\n", count_total_chars(text));
    find_text(text, "the");
    show_word_frequency(text);

    printf("请输入多个关键词（空格分隔）：");
    char input[MAX_LINE_LEN];
    fgets(input, MAX_LINE_LEN, stdin);
    char* nl = strchr(input, '\n');
    if (nl) *nl = '\0';
    highlight_keywords(text, input);

    delete_line(text, 2);
    display_text(text);
    undo_delete(text);
    display_text(text);

    free_graph(text);
    return 0;
}
