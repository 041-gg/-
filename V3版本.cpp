#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 1024
#define MAX_STACK_SIZE 100

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
} TextData;

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

// 显示文本
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
        printf("无操作可撤销\n");
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

// ==================== V3 新增：统计与搜索 ====================

// 1. 统计总行数
int count_total_lines(const TextData* td) {
    if (!td) return 0;
    return td->line_count;
}

// 2. 统计总字符数（不含换行）
int count_total_chars(const TextData* td) {
    if (!td) return 0;
    int total = 0;
    for (int i = 0; i < td->line_count; i++) {
        total += strlen(td->lines[i]);
    }
    return total;
}

// 3. 搜索关键词 BF 算法，输出行号与位置
void find_text(const TextData* td, const char* keyword) {
    if (!td || !keyword || strlen(keyword) == 0) {
        printf("搜索内容无效\n");
        return;
    }

    int found = 0;
    int key_len = strlen(keyword);

    printf("\n===== 搜索结果：关键词「%s」=====\n", keyword);
    for (int i = 0; i < td->line_count; i++) {
        const char* line = td->lines[i];
        int line_len = strlen(line);

        // BF 暴力匹配
        for (int j = 0; j <= line_len - key_len; j++) {
            int match = 1;
            for (int k = 0; k < key_len; k++) {
                if (line[j + k] != keyword[k]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                printf("第 %d 行，第 %d 列\n", i + 1, j + 1);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("未找到匹配内容\n");
    }
    printf("==================================\n\n");
}

// ==================== 主函数测试 ====================
int main() {
    TextData* text = load_file("test.txt");
    if (!text) {
        printf("请先创建 test.txt 文件\n");
        return 1;
    }

    display_text(text);

    // V3 统计功能
    printf("总行数：%d\n", count_total_lines(text));
    printf("总字符数：%d\n", count_total_chars(text));

    // V3 搜索功能
    find_text(text, "行");

    // 原有删除+撤销
    delete_line(text, 2);
    display_text(text);

    undo_delete(text);
    display_text(text);

    free_text(text);
    return 0;
}
