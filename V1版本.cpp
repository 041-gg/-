#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

using namespace std;

struct TextData {
    char* content;
    size_t length;
    bool is_loaded;
};

TextData* load_file(const char* file_path) {
    ifstream file(file_path, ios::binary | ios::ate);
    if (!file.is_open()) {
        return nullptr;
    }

    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    char* buf = new(nothrow) char[size + 1];
    if (!buf) {
        return nullptr;
    }

    if (!file.read(buf, size)) {
        delete[] buf;
        return nullptr;
    }
    buf[size] = '\0';

    TextData* td = new(nothrow) TextData;
    if (!td) {
        delete[] buf;
        return nullptr;
    }

    td->content = buf;
    td->length = static_cast<size_t>(size);
    td->is_loaded = true;
    return td;
}

void display_text(const TextData* text_data) {
    if (!text_data || !text_data->is_loaded || !text_data->content) {
        cout << "[无有效文本可显示]" << endl;
        return;
    }
    cout << "===== 文本内容 =====" << endl;
    cout << text_data->content << endl;
    cout << "====================" << endl;
}

void free_text(TextData* text_data) {
    if (text_data) {
        delete[] text_data->content;
        delete text_data;
    }
}

int main() {
    const char* path = "test.txt";
    TextData* data = load_file(path);

    if (!data) {
        cout << "文件加载失败：" << path << endl;
        return 1;
    }

    display_text(data);
    free_text(data);
    data = nullptr;

    return 0;
}
