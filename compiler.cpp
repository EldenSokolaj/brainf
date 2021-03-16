#include <fstream>
#include <iostream>
#include <stack>

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::stack;
using std::system;

string val(int in){
    if(in == 0){
        return "a";
    }
    string ret = "";
    while(in != 0){
        ret = (char)(in % 26 + 97) + ret;
        in /= 26;
    }
    return ret;
}

int main(int argc, char *argv[]){
    //check for correct number of arguments
    bool del = true;

    if(argc != 3){
        if(argc == 4 && string(argv[3]) == "asm"){
            del = false;
        } else {
            cout << "Usage: " << argv[0] << " inputfile outputfile" << endl;
            return 0;
        }
    }
    
    //setup variables used
    int id = 0;
    stack <int> scope;
    string buffer;
    char buf;
    int opt;
    ifstream file(string(argv[1]).c_str());
    ofstream out( (string(argv[2]) + ".s").c_str());
    
    //validate input file
    if(!file.is_open()){
        cout << "Invalid input file!" << endl;
        return 1;
    }
    
    //setup header
    out << "section .bss\n\tmem: resb 1000\n\nsection .text\n\tglobal _start\n\n\t_start:\n\t\tmov rdi, 126\n\t\tmov rax, mem\n\t\tjmp .end_loop\n\t\t.start_loop:\n\t\t\tmov qword [rax], 0\n\t\t\tadd rax, 8\n\t\t.end_loop:\n\t\t\tdec rdi\n\t\t\tjnz .start_loop\n\t\txor rax, rax\n\t\tmov rsi, mem\n\t\tjmp _main\n\n\t_main:\n\t\t";

    //loop through entire contents of file
    while(true){
        file >> buf;
        if(file.eof()){
            break;
        }

        switch(buf){
            case '+':
                opt = 1;
                while(file.peek() == buf){
                    file >> buf;
                    opt++;
                }
                out << "add byte [rsi], " << opt << "\n\t\t";
                break;
            
            case '-':
                opt = 1;
                while(file.peek() == buf){
                    file >> buf;
                    opt++;
                }
                out << "sub byte [rsi], " << opt << "\n\t\t";
                break;
            
            case '<':
                opt = 1;
                while(file.peek() == buf){
                    file >> buf;
                    opt++;
                }
                out << "sub rsi, " << opt << "\n\t\t";
                break;
            
            case '>':
                opt = 1;
                while(file.peek() == buf){
                    file >> buf;
                    opt++;
                }
                out << "add rsi, " << opt << "\n\t\t";
                break;
            
            case '[':
                id++;
                buffer = val(id);
                scope.push(id);
                out << "jmp .end_" << buffer << "\n\t\t.start_" << buffer << ":\n\t\t";
                break;
            
            case ']':
                buffer = val(scope.top());
                scope.pop();
                out << ".end_" << buffer << ":\n\t\tcmp byte [rsi], 0\n\t\tjne .start_" << buffer << "\n\t\t";
                break;
            
            case '.':
                out << "call print\n\t\t";
                break;
            
            case ',':
                out << "call get\n\t\t";
                break;
        }
    }

    //footer
    out << "jmp exit\n\n\tprint:\n\t\tmov rdx, 1\n\t\tmov rax, 1\n\t\tmov rdi, 1\n\t\tsyscall\n\t\tret\n\n\tget:\n\t\tmov rbx, rsi\n\t\tsub rsp, 2\n\t\tmov rsi, rsp\n\t\tmov rax, 0\n\t\tmov rdi, 0\n\t\tmov rdx, 2\n\t\tsyscall\n\t\tmov al, [rsp]\n\t\tmov rsi, rbx\n\t\tmov [rsi], al\n\t\tadd rsp, 2\n\t\tret\n\n\texit:\n\t\tmov rax, 60\n\t\tsyscall\n";

    //finish up
    file.close();
    out.close();

    string outfile = string(argv[2]);

    system( ("nasm -felf64 " + outfile + ".s").c_str());
    system( ("ld " + outfile + ".o -o " + outfile).c_str() );
    system( ("rm " + outfile + ".o").c_str() );

    if(del){
        system( ("rm " + outfile + ".s").c_str() );
    }

    return 0;
}
