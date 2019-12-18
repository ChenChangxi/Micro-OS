//这是操作系统的课程设计
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <queue>
#include <map>

//一共有6个命令
#define COMMAND 6
#define BEGIN "1"
#define END "2"

using namespace std;

//文件的数据结构
typedef struct file_node {
    string file_name;
    vector<string> file_content;  //采用字符串数组来存放文件内容
    struct file_node *next_file;  //文件列表的下一个节点
} file_node,*file_ptr;

//目录的数据结构
typedef struct dir_node {
    string dir_name;
    unsigned int total_num;  //目录和文件的总数目
    struct dir_node *next_dir;   //本级目录下面的下一个目录
    struct dir_node *dir_list;  //该目录下面的目录队列的第一个目录
    struct file_node *file_list;  //该目录下面的文件队列的第一个文件
} dir_node,*dir_root;

string device; //当前设备
string admin; //当前用户名
dir_root root; //整个文件系统的根目录
map<string,int> operation_type; //操作对应的数字

//这是一些工具函数
queue<string> split(string path,char symbol); //分割字符串
dir_root search_path(dir_root node,queue<string> path);

//递归查询目录
dir_root search_path(dir_root present_root,queue<string> path) {
    if(path.empty()) {
        return present_root;
    } else {
        string dir_name=path.front();
        path.pop();
        dir_root dir_list=present_root->dir_list->next_dir;
        while(dir_list!=nullptr) {
            if(dir_list->dir_name==dir_name+"(d)") {
                return search_path(dir_list, path);
            } else dir_list=dir_list->next_dir;
        }
        return nullptr;
    }
}

//分割字符串
queue<string> split(string path,char symbol) {
    int begin=0;
    queue<string> dirs;
    while((int)path.find(symbol,begin)>0) { //可能返回的是unsigned int，要强转一下。
        int index=path.find(symbol,begin); //index是‘/’所在位置，index的值就是它前面串的“长度”。
        string dir_name=path.substr(begin,index-begin);
        dirs.push(dir_name);
        begin=index+1;
    }
    dirs.push(path.substr(begin,path.length()-begin));  //最后的目录也要进入队列
    return dirs;
}

//在操作系统启动前装入操作系统的命令，按照linux系统来的
void boot() {
    device="MackBook Pro";
    //cd为改变目录
    operation_type.insert(pair<string,int>("cd",1));
    //ls为列出目录的子目录及文件
    operation_type.insert(pair<string,int>("ls",2));
    //md为创建目录
    operation_type.insert(pair<string,int>("md",3));
    //rd为删除目录
    operation_type.insert(pair<string,int>("rd",4));
    //vim为创建文件
    operation_type.insert(pair<string,int>("vim",5));
    //rf为删除文件
    operation_type.insert(pair<string,int>("rf",6));
    //exit为退出系统
    operation_type.insert(pair<string,int>("exit",7));
}

//开始界面
string welcome() {
    string operation;
    cout<<"欢迎来到Micro OS操作系统\n";
    cout<<"请输入用户名\n";
    cin>>admin;
    cout<<"请选择你想要执行的操作\n";
    cout<<"1:进入root 2:退出\n";
    cin>>operation;
    return operation;
}

//切换目录
//这里注意指针和引用的区别，假如参数是指针的话，你能改变的是
//指针指向的内存单元的值，你无法改变main函数中某个变量的值，
//而传引用的作用就是在当前函数中改变main函数中某个变量的值。
void change_dir(dir_root &present_root,string path) { //相对或者绝对路径
    if(path=="cd") {
        present_root=root;
        return;
    }
    queue<string> path_para=split(path,'/');
    if(path[0]=='/') {  //绝对路径
        path=path.substr(1,path.length()-1);   //要去掉第一个‘/’
        if(search_path(root, path_para)==nullptr) {
            cout<<"没有这个文件夹";
        } else present_root=search_path(root, path_para);
    } else {  //相对路径
        if(search_path(present_root, path_para)==nullptr) {
            cout<<"没有这个文件夹";
        } else present_root=search_path(present_root, path_para);
    }
}

//展现目录下面的文件和子目录
void show_dir(dir_root present_root) {
    if(present_root->dir_list!=nullptr) {
        dir_root dir_list=present_root->dir_list;
        while(dir_list->next_dir!=nullptr) {  //注意，无论是文件还是目录的头节点都是占位的
            cout<<dir_list->next_dir->dir_name<<'\n';
            dir_list=dir_list->next_dir;
        }
    }
    if(present_root->file_list!=nullptr) {
        file_ptr file_list=present_root->file_list;
        while(file_list->next_file!=nullptr) {
            cout<<file_list->next_file->file_name<<'\n';
            file_list=file_list->next_file;
        }
    }
}

//创建目录
void make_dir(dir_root present_dir,string dir_name) {
    dir_root new_dir=new dir_node();
    new_dir->dir_name=dir_name;
    new_dir->total_num=0;
    //假如文件夹为空，必须再创建一个文件链表的头节点。
    if(present_dir->dir_list==nullptr) {
        present_dir->dir_list=new dir_node();
    }
    dir_root dir_list=present_dir->dir_list;
    while(dir_list->next_dir!=nullptr) {
        dir_list=dir_list->next_dir;
    }
    dir_list->next_dir=new_dir;
}

//删除目录
void delete_dir(dir_root present_dir,string dir_name) {
    dir_root temp=present_dir->dir_list;   //删除链表节点的时候必须有指针指向头节点
    dir_root dir_list=temp->next_dir;
    while(dir_list!=nullptr) {
        if(dir_list->dir_name==dir_name) {
            temp->next_dir=dir_list->next_dir;
            break;
        }
        temp=temp->next_dir;
        dir_list=temp->next_dir;
    }
    if(dir_list==nullptr) {
        cout<<"没有这个文件夹\n";
    }
    delete dir_list; //释放内存
}

//查看或者创建文件
void make_file(dir_root present_dir,string file_name) {
    if(present_dir->file_list==nullptr) {
        present_dir->file_list=new file_node();
    }
    file_ptr file_list=present_dir->file_list;
    while(file_list->next_file!=nullptr&&
          file_list->next_file->file_name!=file_name) {
        file_list=file_list->next_file;
    }
    cout<<"请输入文件内容:\n";
    cout<<"输入完成后，按1保存，按2退出\n";
    vector<string> temp;
    if(file_list->next_file) {
        if(file_list->next_file->file_name==file_name) {   //这个文件已经存在，操作是查看
            temp=file_list->next_file->file_content;
            for(int i=0;i<temp.size();++i) {
                cout<<temp[i]<<'\n';
            }
        }
    } else {
        file_ptr new_file=new file_node();
        new_file->file_name=file_name;
        file_list->next_file=new_file;
    }
    string content;
    while(cin>>content) {   //cin会从缓冲区读数据，并把换行符清除出缓冲区
        if(content=="1") {
            file_list->next_file->file_content=temp;
            break;
        } else if(content=="2") break;
        else temp.push_back(content);
    }
}

//删除文件
void delete_file(dir_root present_dir,string file_name) {
    file_ptr temp=present_dir->file_list;
    file_ptr file_list=temp->next_file;
    while(file_list!=nullptr) {
        if(file_list->file_name==file_name) {
            temp->next_file=file_list->next_file;
            break;
        }
        temp=temp->next_file;
        file_list=temp->next_file;
    }
    if(file_list==nullptr) {
        cout<<"没有这个文件\n";
    }
    delete file_list;
}

int main() {
    boot();
    //开始界面
    string command=welcome();
    if(command==BEGIN) {
        //如果启动，则创建根目录
        root=new dir_node();
        root->dir_name="root";
        root->total_num=0;
    } else if(command==END) {
        return 0;
    }
    //当前在根目录
    int flag=0;
    dir_root presert_dir=root;
    while(true) {
        string prefix;
        if(presert_dir->dir_name=="root") {
            prefix=device+":~ "+admin+"$";
        } else {
            int index=presert_dir->dir_name.find('(');
            string dir_name=presert_dir->dir_name.substr(0,index);
            prefix=device+":"+dir_name+" "+admin+"$";
        }
        cout<<prefix;
        string input,command,parameter;
        //getline遇到换行符会读换行符，读完后把剩余的换行符清除出缓冲区
        //cin遇到换行符会清除换行符而不读，而读真正的数据，读完后不清除剩
        //余的换行符。
        if(flag==0) {getchar();flag=1;}
        getline(cin,input);
        queue<string> parts=split(input,' ');
        command=parts.front();
        parameter=parts.back();
        switch (operation_type.at(command)) {
            case 1:
                change_dir(presert_dir, parameter);break;
            case 2:
                show_dir(presert_dir);break;
            case 3:
                make_dir(presert_dir,parameter+"(d)");break;
            case 4:
                delete_dir(presert_dir,parameter+"(d)");break;
            case 5:
                flag=0;   //输入文件会额外多打一个换行符
                make_file(presert_dir,parameter+"(f)");break;
            case 6:
                delete_file(presert_dir,parameter+"(f)");break;
            case 7:
                return 0;
        }
    }
}
