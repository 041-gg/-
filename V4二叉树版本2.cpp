#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 1024
#define MAX_STACK_SIZE 100
#define MAX_WORD_LEN 100

// 文本结构体
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
} TextData;

// 二叉树节点：存单词+词频
typedef struct TreeNode {
    char word[MAX_WORD_LEN];
    int count;
    struct TreeNode *left, *right;
} TreeNode;

TextData undo_stack[MAX_STACK_SIZE];
int stack_top = -1;

// ========== 文本基础功能（不变） ==========
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

// ========== 统计行数、字符数（不变） ==========
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

// ========== BF关键词搜索（不变） ==========
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

// ========== 二叉树词频模块（全新改写） ==========
// 转小写
void to_lower(char* s) {
    for (; *s; s++) *s = tolower(*s);
}

// 判断是否字母
int is_valid_char(int c) {
    return isalpha(c);
}

// 提取单个单词
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

// 创建二叉树新节点
TreeNode* create_node(const char* word) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    strcpy(node->word, word);
    node->count = 1;
    node->left = node->right = NULL;
    return node;
}

// 二叉搜索树插入单词：存在则计数+1，不存在新建节点
TreeNode* insert_tree(TreeNode* root, const char* word) {
    if (root == NULL) {
        return create_node(word);
    }
    int cmp = strcmp(word, root->word);
    if (cmp == 0) {
        root->count++;
    } else if (cmp < 0) {
        root->left = insert_tree(root->left, word);
    } else {
        root->right = insert_tree(root->right, word);
    }
    return root;
}

// 中序遍历二叉树 → 字典序输出词频
void inorder_print(TreeNode* root) {
    if (!root) return;
    inorder_print(root->left);
    printf("%-15s %d\n", root->word, root->count);
    inorder_print(root->right);
}

// 释放二叉树内存
void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// 统计全文单词并构建二叉树
void show_word_frequency_tree(const TextData* td) {
    if (!td || td->line_count == 0) {
        printf("无文本可统计\n");
        return;
    }
    TreeNode* root = NULL;
    char word[MAX_WORD_LEN];

    for (int i = 0; i < td->line_count; i++) {
        const char* p = td->lines[i];
        while (get_word(&p, word)) {
            to_lower(word);
            root = insert_tree(root, word);
        }
    }

    printf("\n===== 二叉树词频统计（字典序）=====\n");
    inorder_print(root);
    printf("==================================\n\n");

    free_tree(root);
}

// ========== 主函数 ==========
int main() {
    TextData* text = load_file("D:\\test.txt");
    if (!text) return 1;

    display_text(text);
    printf("总行数：%d\n", count_total_lines(text));
    printf("总字符数：%d\n", count_total_chars(text));

    find_text(text, "the");
    show_word_frequency_tree(text);

    delete_line(text, 2);
    display_text(text);
    undo_delete(text);
    display_text(text);

    free_text(text);
    return 0;
}
