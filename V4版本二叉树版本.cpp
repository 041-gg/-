#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINE_LEN 1024
#define MAX_WORDS 5000
#define MAX_WORD_LEN 100
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;
typedef struct TreeNode {
    int line_no;                
    char content[MAX_LINE_LEN];  
    struct TreeNode *lchild, *rchild;
} TreeNode, *TextTree;

typedef struct UndoNode {
    TextTree root;
    struct UndoNode* next;
} UndoNode;
UndoNode* undo_top = NULL;
TreeNode* createNode(int line_no, const char* content) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->line_no = line_no;
    strcpy(node->content, content);
    node->lchild = node->rchild = NULL;
    return node;
}
void treeInsert(TextTree* T, int line_no, const char* content) {
    if (*T == NULL) {
        *T = createNode(line_no, content);
        return;
    }
    if (line_no < (*T)->line_no)
        treeInsert(&(*T)->lchild, line_no, content);
    else
        treeInsert(&(*T)->rchild, line_no, content);
}
void treeInOrder(TreeTree T) {
    if (!T) return;
    treeInOrder(T->lchild);
    printf("%d: %s\n", T->line_no, T->content);
    treeInOrder(T->rchild);
}
TreeNode* treeSearch(TextTree T, int line_no) {
    if (!T) return NULL;
    if (T->line_no == line_no) return T;
    if (line_no < T->line_no)
        return treeSearch(T->lchild, line_no);
    else
        return treeSearch(T->rchild, line_no);
}
TreeNode* findMinNode(TextTree T) {
    while (T->lchild) T = T->lchild;
    return T;
}
int treeDelete(TextTree* T, int line_no) {
    if (!*T) return 0;
    if (line_no < (*T)->line_no) {
        return treeDelete(&(*T)->lchild, line_no);
    } else if (line_no > (*T)->line_no) {
        return treeDelete(&(*T)->rchild, line_no);
    } else {
        TreeNode* p = *T;
        if (!p->lchild) {
            *T = p->rchild;
        } else if (!p->rchild) {
            *T = p->lchild;
        } else {
            TreeNode* min = findMinNode(p->rchild);
            strcpy(p->content, min->content);
            p->line_no = min->line_no;
            treeDelete(&p->rchild, min->line_no);
        }
        free(p);
        return 1;
    }
}
int treeCountLines(TextTree T) {
    if (!T) return 0;
    return 1 + treeCountLines(T->lchild) + treeCountLines(T->rchild);
}
int treeCountChars(TextTree T) {
    if (!T) return 0;
    return strlen(T->content) 
         + treeCountChars(T->lchild) 
         + treeCountChars(T->rchild);
}
TextTree copyTree(TreeNode* src) {
    if (!src) return NULL;
    TreeNode* dest = createNode(src->line_no, src->content);
    dest->lchild = copyTree(src->lchild);
    dest->rchild = copyTree(src->rchild);
    return dest;
}
void freeTree(TextTree T) {
    if (!T) return;
    freeTree(T->lchild);
    freeTree(T->rchild);
    free(T);
}
void saveUndo(TextTree root) {
    UndoNode* newNode = (UndoNode*)malloc(sizeof(UndoNode));
    newNode->root = copyTree(root);
    newNode->next = undo_top;
    undo_top = newNode;
}
int undoOp(TextTree* T) {
    if (!undo_top) {
        printf("无操作可撤销\n");
        return 0;
    }
    freeTree(*T);
    *T = undo_top->root;
    UndoNode* temp = undo_top;
    undo_top = undo_top->next;
    free(temp);
    printf("已撤销上一次删除\n");
    return 1;
}
TextTree loadFileToTree(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (!fp) {
        printf("文件打开失败\n");
        return NULL;
    }
    TextTree T = NULL;
    char buf[MAX_LINE_LEN];
    int line_no = 1;
    while (fgets(buf, MAX_LINE_LEN, fp)) {
        char* nl = strchr(buf, '\n');
        if (nl) *nl = '\0';
        treeInsert(&T, line_no++, buf);
    }
    fclose(fp);
    return T;
}
void displayTreeText(TextTree T) {
    if (!T) {
        printf("[无文本内容]\n");
        return;
    }
    printf("\n===== 文本内容 =====\n");
    treeInOrder(T);
    printf("====================\n\n");
}
int deleteLineByTree(TextTree* T, int line_no) {
    if (!treeSearch(*T, line_no)) {
        printf("无效行号\n");
        return 0;
    }
    saveUndo(*T);     
    treeDelete(T, line_no);
    printf("已删除第 %d 行\n", line_no);
    return 1;
}

void bfSearchTree(TextTree T, const char* keyword) {
    if (!T || !keyword || !*keyword) {
        printf("搜索内容无效\n");
        return;
    }
    int key_len = strlen(keyword);
    int found = 0;
    printf("\n===== 搜索「%s」=====\n", keyword);
    void matchNode(TreeNode* node) {
        if (!node) return;
        const char* s = node->content;
        int len = strlen(s);
        for (int j = 0; j <= len - key_len; j++) {
            int ok = 1;
            for (int k = 0; k < key_len; k++) {
                if (s[j+k] != keyword[k]) { ok = 0; break; }
            }
            if (ok) {
                printf("第 %d 行 第 %d 列\n", node->line_no, j+1);
                found = 1;
            }
        }
        matchNode(node->lchild);
        matchNode(node->rchild);
    }
    matchNode(T);
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

int cmp_freq(const void* a, const void* b) {
    WordFreq* w1 = (WordFreq*)a;
    WordFreq* w2 = (WordFreq*)b;
    return w2->count - w1->count;
}
void wordFreqTree(TextTree T) {
    WordFreq freq_list[MAX_WORDS];
    memset(freq_list, 0, sizeof(freq_list));
    int word_num = 0;

    void travelWord(TreeNode* node) {
        if (!node) return;
        const char* p = node->content;
        char word[MAX_WORD_LEN];
        while (get_word(&p, word)) {
            to_lower(word);
            int exist = 0;
            for (int j = 0; j < word_num; j++) {
                if (strcmp(freq_list[j].word, word) == 0) {
                    freq_list[j].count++;
                    exist = 1;
                    break;
                }
            }
            if (!exist && word_num < MAX_WORDS) {
                strcpy(freq_list[word_num].word, word);
                freq_list[word_num].count = 1;
                word_num++;
            }
        }
        travelWord(node->lchild);
        travelWord(node->rchild);
    }

    travelWord(T);
    if (word_num == 0) {
        printf("无单词可统计\n");
        return;
    }
    qsort(freq_list, word_num, sizeof(WordFreq), cmp_freq);
    printf("\n===== 词频统计（降序）=====\n");
    for (int i = 0; i < word_num; i++) {
        printf("%-15s %d\n", freq_list[i].word, freq_list[i].count);
    }
    printf("==========================\n\n");
}
int main() {
    TextTree textTree = loadFileToTree("test.txt");
    if (!textTree) return 1;
    displayTreeText(textTree);
    printf("总行数：%d\n", treeCountLines(textTree));
    printf("总字符数：%d\n", treeCountChars(textTree));

    bfSearchTree(textTree, "the");
    wordFreqTree(textTree);
    deleteLineByTree(&textTree, 2);
    displayTreeText(textTree);

    undoOp(&textTree);
    displayTreeText(textTree);
    freeTree(textTree);
    return 0;
}
