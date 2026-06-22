#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 1024
#define MAX_STACK_SIZE 100
#define MAX_WORDS 5000    // 最大单词存储量
#define MAX_WORD_LEN 100  // 单词最大长度
#define MAX_KEYWORDS 20   // 最大支持的关键词数量
#define COLOR_RED "\033[31m"  // 红色高亮
#define COLOR_RESET "\033[0m" // 重置颜色

// 文本数据结构
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
} TextData;

// 词频结构
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;

TextData undo_stack[MAX_STACK_SIZE];
int stack_top = -1;

// 加载文件
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

// 显示原始文本
void display_text(const TextData* td) {
    if (!td || td->line_count == 0) {
        printf("[文本为空]\n");
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

// 保存状态
int save_state(TextData* td) {
    if (stack_top >= MAX_STACK_SIZE - 1) {
        printf("撤销栈已满\n");
        return 0;
    }
    stack_top++;
    memcpy(&undo_stack[stack_top], td, sizeof(TextData));
    return 1;
}

// 撤销
int undo_delete(TextData* td) {
    if (stack_top < 0) {
        printf("无可撤销操作\n");
        return 0;
    }
    memcpy(td, &undo_stack[stack_top], sizeof(TextData));
    stack_top--;
    printf("已撤销上一次删除\n");
    return 1;
}

// 删除行
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

// ==================== V3 统计查询功能 ====================

// 统计总行数
int count_total_lines(const TextData* td) {
    return td ? td->line_count : 0;
}

// 统计字符数
int count_total_chars(const TextData* td) {
    if (!td) return 0;
    int total = 0;
    for (int i = 0; i < td->line_count; i++)
        total += strlen(td->lines[i]);
    return total;
}

// BF 查找单关键词
void find_text(const TextData* td, const char* keyword) {
    if (!td || !keyword || !*keyword) {
        printf("查询参数无效\n");
        return;
    }

    int found = 0;
    int key_len = strlen(keyword);
    printf("\n===== 查找'%s' =====\n", keyword);

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
                printf("在 %d 行 第 %d 列\n", i + 1, j + 1);
                found = 1;
            }
        }
    }
    if (!found) printf("未找到\n");
    printf("====================\n\n");
}

// ==================== V4 词频统计功能 ====================

// 转为小写
void to_lower(char* s) {
    for (; *s; s++) *s = tolower(*s);
}

// 判断是否为字母
int is_valid_char(int c) {
    return isalpha(c);
}

// 提取单词
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

// 词频统计
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

// qsort 比较函数（按词频降序）
int cmp_freq(const void* a, const void* b) {
    WordFreq* w1 = (WordFreq*)a;
    WordFreq* w2 = (WordFreq*)b;
    return w2->count - w1->count;
}

// 排序词频
void sort_frequency(WordFreq* list, int n) {
    qsort(list, n, sizeof(WordFreq), cmp_freq);
}

// 显示词频
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

// ==================== V5 多关键词高亮功能 ====================

// 分割多关键词（空格分隔）
int split_keywords(const char* input, char keywords[MAX_KEYWORDS][MAX_WORD_LEN]) {
    if (!input || !*input) return 0;

    int count = 0;
    const char* p = input;
    while (*p && count < MAX_KEYWORDS) {
        // 跳过空格
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        // 提取关键词
        int i = 0;
        while (*p && !isspace((unsigned char)*p) && i < MAX_WORD_LEN - 1) {
            keywords[count][i++] = *p++;
        }
        keywords[count][i] = '\0';
        count++;
    }
    return count;
}

// 多关键词高亮核心函数
void highlight_keywords(const TextData* td, const char* input_keywords) {
    if (!td || !input_keywords || !*input_keywords) {
        printf("关键词参数无效\n");
        return;
    }

    // 分割多关键词
    char keywords[MAX_KEYWORDS][MAX_WORD_LEN];
    int keyword_count = split_keywords(input_keywords, keywords);
    if (keyword_count == 0) {
        printf("未识别到有效关键词\n");
        return;
    }

    printf("\n===== 多关键词高亮展示 =====\n");
    // 遍历每一行文本
    for (int line_idx = 0; line_idx < td->line_count; line_idx++) {
        const char* line = td->lines[line_idx];
        int line_len = strlen(line);
        char highlighted_line[MAX_LINE_LEN * 2] = {0};  // 预留高亮字符空间
        int pos = 0;  // 高亮行的写入位置

        // 遍历当前行的每个字符
        for (int i = 0; i < line_len; ) {
            int matched = 0;
            // 匹配所有关键词
            for (int k = 0; k < keyword_count; k++) {
                int key_len = strlen(keywords[k]);
                // 检查是否匹配当前位置
                if (i + key_len <= line_len && strncmp(&line[i], keywords[k], key_len) == 0) {
                    // 写入高亮前缀
                    strcpy(&highlighted_line[pos], COLOR_RED);
                    pos += strlen(COLOR_RED);
                    // 写入关键词
                    strncpy(&highlighted_line[pos], &line[i], key_len);
                    pos += key_len;
                    // 写入重置颜色
                    strcpy(&highlighted_line[pos], COLOR_RESET);
                    pos += strlen(COLOR_RESET);
                    // 跳过已匹配的字符
                    i += key_len;
                    matched = 1;
                    break;
                }
            }
            // 未匹配则直接写入原字符
            if (!matched) {
                highlighted_line[pos++] = line[i++];
            }
        }

        // 输出高亮后的行
        printf("%d: %s\n", line_idx + 1, highlighted_line);
    }
    printf("===========================\n\n");
}

// ==================== 主函数 ====================
int main() {
    TextData* text = load_file("test.txt");
    if (!text) return 1;

    // 显示原始文本
    display_text(text);

    // V3 基础统计
    printf("总行数：%d\n", count_total_lines(text));
    printf("总字符数：%d\n", count_total_chars(text));

    // 单关键词查找
    find_text(text, "the");

    // V4 词频统计
    show_word_frequency(text);

    // V5 多关键词高亮演示
    printf("请输入多个关键词（空格分隔）：");
    char input[MAX_LINE_LEN];
    fgets(input, MAX_LINE_LEN, stdin);
    // 去除换行符
    char* nl = strchr(input, '\n');
    if (nl) *nl = '\0';
    // 高亮展示
    highlight_keywords(text, input);

    // 删除行与撤销演示
    delete_line(text, 2);
    display_text(text);

    undo_delete(text);
    display_text(text);

    free_text(text);
    return 0;
}
