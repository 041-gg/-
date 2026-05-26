#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 1024
#define MAX_STACK_SIZE 100
#define MAX_WORDS 5000    
#define MAX_WORD_LEN 100  

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
} TextData;
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;

TextData undo_stack[MAX_STACK_SIZE];
int stack_top = -1;
TextData* load_file(const char* file_path) {
    TextData* td = (TextData*)malloc(sizeof(TextData));
    td->line_count = 0;

    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        printf("文件打开失败\n");
        free(td);
        return NULL;
    }

    while (td->line_count < MAX_LINES &&
           fgets(td->lines[td->line_count], MAX_LINE_LEN, fp)) {
        char* nl = strchr(td->lines[td->line_count], '\n');
        if (nl) *nl = '\0';
        td->line_count++;
    }

    fclose(fp);
    return td;
}

void display_text(const TextData* td) {
    if (!td || td->line_count == 0) {
        printf("[无文本内容]\n");
        return;
    }
    printf("\n===== 文本内容 =====\n");
    for (int i = 0; i < td->line_count; i++) {
        printf("%d: %s\n", i + 1, td->lines[i]);
    }
    printf("====================\n\n");
}

void free_text(TextData* td) {
    if (td) free(td);
}
int save_state(TextData* td) {
    if (stack_top >= MAX_STACK_SIZE - 1) {
        printf("撤销栈已满\n");
        return 0;
    }
    stack_top++;
    memcpy(&undo_stack[stack_top], td, sizeof(TextData));
    return 1;
}
int undo_delete(TextData* td) {
    if (stack_top < 0) {
        printf("无操作可撤销\n");
        return 0;
    }
    memcpy(td, &undo_stack[stack_top], sizeof(TextData));
    stack_top--;
    printf("已撤销上一次删除\n");
    return 1;
}
int delete_line(TextData* td, int line_num) {
    if (!td || line_num < 1 || line_num > td->line_count) {
        printf("无效行号\n");
        return 0;
    }
    if (!save_state(td)) return 0;

    int idx = line_num - 1;
    for (int i = idx; i < td->line_count - 1; i++) {
        strcpy(td->lines[i], td->lines[i + 1]);
    }
    td->line_count--;
    printf("已删除第 %d 行\n", line_num);
    return 1;
} 
int count_total_lines(const TextData* td) {
    return td ? td->line_count : 0;
}
int count_total_chars(const TextData* td) {
    if (!td) return 0;
    int total = 0;
    for (int i = 0; i < td->line_count; i++)
        total += strlen(td->lines[i]);
    return total;
}
void find_text(const TextData* td, const char* keyword) {
    if (!td || !keyword || !*keyword) {
        printf("搜索内容无效\n");
        return;
    }

    int found = 0;
    int key_len = strlen(keyword);
    printf("\n===== 搜索「%s」=====\n", keyword);

    for (int i = 0; i < td->line_count; i++) {
        const char* s = td->lines[i];
        int len = strlen(s);
        for (int j = 0; j <= len - key_len; j++) {
            int match = 1;
            for (int k = 0; k < key_len; k++) {
                if (s[j + k] != keyword[k]) {
                    match = 0; break;
                }
            }
            if (match) {
                printf("第 %d 行 第 %d 列\n", i + 1, j + 1);
                found = 1;
            }
        }
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
int count_word_frequency(const TextData* td, WordFreq* freq_list) {
    if (!td) return 0;

    int word_num = 0;
    memset(freq_list, 0, sizeof(WordFreq) * MAX_WORDS);

    for (int i = 0; i < td->line_count; i++) {
        const char* p = td->lines[i];
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
void show_word_frequency(const TextData* td) {
    WordFreq freq_list[MAX_WORDS];
    int n = count_word_frequency(td, freq_list);
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
int main() {
    TextData* text = load_file("D:\\test.txt");
    if (!text) return 1;

    display_text(text);

    printf("总行数：%d\n", count_total_lines(text));
    printf("总字符数：%d\n", count_total_chars(text));

    find_text(text, "the");
    show_word_frequency(text);

    delete_line(text, 2);
    display_text(text);

    undo_delete(text);
    display_text(text);

    free_text(text);
    return 0;
}
