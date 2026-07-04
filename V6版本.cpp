#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 1024
#define MAX_STACK_SIZE 100
#define MAX_WORDS 5000    
#define MAX_WORD_LEN 100  
#define MAX_KEYWORDS 20   
#define COLOR_RED "\033[31m"  
#define COLOR_RESET "\033[0m" 

// 文本数据结构
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
    char file_path[MAX_LINE_LEN];  // 记录当前加载的文件路径
} TextData;

// 词频结构
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;

// 全局变量
TextData undo_stack[MAX_STACK_SIZE];
int stack_top = -1;
TextData* current_text = NULL;  // 当前编辑的文本数据

// ==================== 基础文件操作模块 ====================
// 加载文件
TextData* load_file(const char* file_path) {
    TextData* td = (TextData*)malloc(sizeof(TextData));
    if (!td) {
        printf("内存分配失败\n");
        return NULL;
    }
    td->line_count = 0;
    strncpy(td->file_path, file_path, MAX_LINE_LEN - 1);
    td->file_path[MAX_LINE_LEN - 1] = '\0';

    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        printf("文件打开失败: %s\n", file_path);
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
    printf("成功加载文件: %s (共%d行)\n", file_path, td->line_count);
    return td;
}

// 保存文件
int save_file(const TextData* td, const char* file_path) {
    if (!td) {
        printf("无文本数据可保存\n");
        return 0;
    }

    FILE* fp = fopen(file_path, "w");
    if (!fp) {
        printf("文件保存失败: %s\n", file_path);
        return 0;
    }

    for (int i = 0; i < td->line_count; i++) {
        fprintf(fp, "%s\n", td->lines[i]);
    }

    fclose(fp);
    printf("成功保存文件: %s (共%d行)\n", file_path, td->line_count);
    return 1;
}

// 显示文本
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

// 释放文本内存
void free_text(TextData* td) {
    if (td) {
        free(td);
        td = NULL;
    }
}

// ==================== 编辑操作模块 ====================
// 保存状态到撤销栈
int save_state(TextData* td) {
    if (stack_top >= MAX_STACK_SIZE - 1) {
        printf("撤销栈已满\n");
        return 0;
    }
    stack_top++;
    memcpy(&undo_stack[stack_top], td, sizeof(TextData));
    return 1;
}

// 撤销操作
int undo_delete(TextData* td) {
    if (stack_top < 0) {
        printf("无操作可撤销\n");
        return 0;
    }
    memcpy(td, &undo_stack[stack_top], sizeof(TextData));
    stack_top--;
    printf("已撤销上一次删除操作\n");
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

// 插入行
int insert_line(TextData* td, int line_num, const char* content) {
    if (!td || line_num < 1 || line_num > td->line_count + 1 || !content) {
        printf("无效行号或内容\n");
        return 0;
    }
    if (td->line_count >= MAX_LINES) {
        printf("文本行数已达上限\n");
        return 0;
    }
    if (!save_state(td)) return 0;

    int idx = line_num - 1;
    // 后移行
    for (int i = td->line_count; i > idx; i--) {
        strcpy(td->lines[i], td->lines[i - 1]);
    }
    // 插入新行
    strncpy(td->lines[idx], content, MAX_LINE_LEN - 1);
    td->lines[idx][MAX_LINE_LEN - 1] = '\0';
    td->line_count++;
    printf("已在第 %d 行插入内容\n", line_num);
    return 1;
}

// 修改行
int modify_line(TextData* td, int line_num, const char* content) {
    if (!td || line_num < 1 || line_num > td->line_count || !content) {
        printf("无效行号或内容\n");
        return 0;
    }
    if (!save_state(td)) return 0;

    int idx = line_num - 1;
    strncpy(td->lines[idx], content, MAX_LINE_LEN - 1);
    td->lines[idx][MAX_LINE_LEN - 1] = '\0';
    printf("已修改第 %d 行内容\n", line_num);
    return 1;
}

// ==================== 统计查询模块 ====================
// 统计总行数
int count_total_lines(const TextData* td) {
    return td ? td->line_count : 0;
}

// 统计总字符数
int count_total_chars(const TextData* td) {
    if (!td) return 0;
    int total = 0;
    for (int i = 0; i < td->line_count; i++)
        total += strlen(td->lines[i]);
    return total;
}

// BF算法查找关键词
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
                printf("第 %d 行 第 %d 列\n", i + 1, j + 1);
                found = 1;
            }
        }
    }
    if (!found) printf("未找到\n");
    printf("====================\n\n");
}

// ==================== 词频统计模块 ====================
// 转为小写
void to_lower(char* s) {
    for (; *s; s++) *s = tolower(*s);
}

// 判断是否为字母
int is_valid_char(int c) {
    return isalpha(c);
}

// 获取单词
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
    if (!td) {
        printf("无文本数据可统计\n");
        return;
    }
    
    WordFreq freq_list[MAX_WORDS];
    int n = count_word_frequency(td, freq_list);
    if (n == 0) {
        printf("无单词可统计\n");
        return;
    }

    sort_frequency(freq_list, n);

    printf("\n===== 词频统计（降序）=====\n");
    printf("%-15s %s\n", "单词", "出现次数");
    printf("------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("%-15s %d\n", freq_list[i].word, freq_list[i].count);
    }
    printf("==========================\n\n");
}

// ==================== 关键词高亮模块 ====================
// 分割关键词（空格分隔）
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

// 关键词高亮显示
void highlight_keywords(const TextData* td, const char* input_keywords) {
    if (!td || !input_keywords || !*input_keywords) {
        printf("关键词参数无效\n");
        return;
    }

    // 分割关键词
    char keywords[MAX_KEYWORDS][MAX_WORD_LEN];
    int keyword_count = split_keywords(input_keywords, keywords);
    if (keyword_count == 0) {
        printf("未识别到有效关键词\n");
        return;
    }

    printf("\n===== 关键词高亮显示 =====\n");
    printf("高亮关键词: ");
    for (int i = 0; i < keyword_count; i++) {
        printf("%s ", keywords[i]);
    }
    printf("\n------------------------\n");

    // 处理每一行文本
    for (int line_idx = 0; line_idx < td->line_count; line_idx++) {
        const char* line = td->lines[line_idx];
        int line_len = strlen(line);
        char highlighted_line[MAX_LINE_LEN * 2] = {0};
        int pos = 0;

        // 逐字符处理
        for (int i = 0; i < line_len; ) {
            int matched = 0;
            // 匹配所有关键词
            for (int k = 0; k < keyword_count; k++) {
                int key_len = strlen(keywords[k]);
                if (i + key_len <= line_len && strncmp(&line[i], keywords[k], key_len) == 0) {
                    // 写入红色前缀
                    strcpy(&highlighted_line[pos], COLOR_RED);
                    pos += strlen(COLOR_RED);
                    // 写入关键词
                    strncpy(&highlighted_line[pos], &line[i], key_len);
                    pos += key_len;
                    // 写入重置颜色
                    strcpy(&highlighted_line[pos], COLOR_RESET);
                    pos += strlen(COLOR_RESET);
                    // 移动指针
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

        // 输出高亮行
        printf("%d: %s\n", line_idx + 1, highlighted_line);
    }
    printf("===========================\n\n");
}

// ==================== 模块调度器 ====================
void module_manager(int choice) {
    char input_path[MAX_LINE_LEN];
    char input_content[MAX_LINE_LEN];
    char keyword[MAX_LINE_LEN];
    int line_num;

    switch (choice) {
        // 文件操作
        case 1:
            printf("请输入要加载的文件路径: ");
            fgets(input_path, MAX_LINE_LEN, stdin);
            input_path[strcspn(input_path, "\n")] = '\0';
            if (current_text) free_text(current_text);
            current_text = load_file(input_path);
            break;

        case 2:
            if (!current_text) {
                printf("无文本数据可保存\n");
                break;
            }
            printf("请输入保存的文件路径: ");
            fgets(input_path, MAX_LINE_LEN, stdin);
            input_path[strcspn(input_path, "\n")] = '\0';
            save_file(current_text, input_path);
            break;

        case 3:
            display_text(current_text);
            break;

        // 编辑操作
        case 4:
            if (!current_text) {
                printf("请先加载文件\n");
                break;
            }
            printf("请输入要删除的行号: ");
            scanf("%d", &line_num);
            getchar();  // 清空缓冲区
            delete_line(current_text, line_num);
            break;

        case 5:
            if (!current_text) {
                printf("请先加载文件\n");
                break;
            }
            printf("请输入要插入的行号: ");
            scanf("%d", &line_num);
            getchar();
            printf("请输入插入内容: ");
            fgets(input_content, MAX_LINE_LEN, stdin);
            input_content[strcspn(input_content, "\n")] = '\0';
            insert_line(current_text, line_num, input_content);
            break;

        case 6:
            if (!current_text) {
                printf("请先加载文件\n");
                break;
            }
            printf("请输入要修改的行号: ");
            scanf("%d", &line_num);
            getchar();
            printf("请输入修改内容: ");
            fgets(input_content, MAX_LINE_LEN, stdin);
            input_content[strcspn(input_content, "\n")] = '\0';
            modify_line(current_text, line_num, input_content);
            break;

        case 7:
            if (!current_text) {
                printf("请先加载文件\n");
                break;
            }
            undo_delete(current_text);
            break;

        // 统计查询
        case 8:
            printf("总行数: %d\n", count_total_lines(current_text));
            printf("总字符数: %d\n", count_total_chars(current_text));
            break;

        case 9:
            if (!current_text) {
                printf("请先加载文件\n");
                break;
            }
            printf("请输入要查找的关键词: ");
            fgets(keyword, MAX_LINE_LEN, stdin);
            keyword[strcspn(keyword, "\n")] = '\0';
            find_text(current_text, keyword);
            break;

        case 10:
            show_word_frequency(current_text);
            break;

        case 11:
            if (!current_text) {
                printf("请先加载文件\n");
                break;
            }
            printf("请输入要高亮的关键词（空格分隔）: ");
            fgets(keyword, MAX_LINE_LEN, stdin);
            keyword[strcspn(keyword, "\n")] = '\0';
            highlight_keywords(current_text, keyword);
            break;

        case 0:
            printf("正在退出程序...\n");
            if (current_text) free_text(current_text);
            exit(0);
            break;

        default:
            printf("无效的选择，请重新输入\n");
            break;
    }
}

// ==================== 交互式主菜单 ====================
void interactive_menu() {
    int choice;
    printf("========================================\n");
    printf("      智能文本处理系统 V6 (产品化版)     \n");
    printf("========================================\n");
    
    while (1) {
        printf("\n===== 主菜单 =====\n");
        printf("【文件操作】\n");
        printf("1. 加载文件\n");
        printf("2. 保存文件\n");
        printf("3. 查看文本内容\n");
        printf("【编辑操作】\n");
        printf("4. 删除指定行\n");
        printf("5. 插入指定行\n");
        printf("6. 修改指定行\n");
        printf("7. 撤销上一步删除\n");
        printf("【统计查询】\n");
        printf("8. 统计行数/字符数\n");
        printf("9. 查找关键词位置\n");
        printf("10. 单词词频统计\n");
        printf("11. 关键词高亮显示\n");
        printf("【系统操作】\n");
        printf("0. 退出程序\n");
        printf("==================\n");
        printf("请输入功能编号: ");
        
        // 输入处理
        while (scanf("%d", &choice) != 1) {
            getchar();  // 清空无效输入
            printf("输入无效，请输入数字: ");
        }
        getchar();  // 清空换行符
        
        // 调度功能模块
        module_manager(choice);
        
        printf("\n按回车键继续...");
        getchar();
        system("clear");  // Windows系统请改为 system("cls");
    }
}

// ==================== 主函数 ====================
int main() {
    // 初始化
    current_text = NULL;
    stack_top = -1;
    
    // 启动交互式菜单
    interactive_menu();
    
    // 清理资源
    if (current_text) free_text(current_text);
    return 0;
}
