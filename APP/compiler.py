import os

cpp_file = "operations.cpp"
hpp_file = "tables.hpp"
exe_file = "operations.exe"
def erase_all_tables():
    file = open(hpp_file, 'w').close()
    file = open(hpp_file, 'a')
    file.write("#include \"Types.hpp\"\n")
    file.close()

def create_table(args):
    file = open(hpp_file, 'a')
    name = args[1]
    structure = args[2]
    structure_templ = args[3]
    key_type = args[4]
    key_name = args[5]
    file.write("struct "+name+"{\n")
    i = 4
    while i<len(args):
        file.write(args[i] + ' ' + args[i + 1] + ";\n")
        i += 2
    file.write(key_type+" get_key(void)const{return "+key_name+";}\n};\nstd::ostream& operator<<(std::ostream& os, const "+name+"& data){\nos")
    i = 5
    while i < len(args):
        file.write("<<data."+args[i]+"<<\"\\t\"")
        i += 2
    file.write(";\nreturn os;\n}\n"+structure+"<"+name+","+key_type+","+structure_templ+">"+name+"_ (\""+name+"\");\n")
    file.close()
def recreate_upper_operations_file():
    file = open(cpp_file, 'w').close()
    file = open(cpp_file, 'a')
    file.write('#include "' + hpp_file + '"\n#include <chrono>\n#include <thread>\n#include <functional>\n#include <sstream>\n#include <mutex>\n\nstd::mutex mtx;\n\n')
    file.close()
def recreate_upper_operations_table(name):
    file = open(cpp_file, 'a')
    file.write("void "+ name +"_operations(void){\nstd::vector<std::function<void(void)>> writes;\nstd::vector<std::function<void(std::stringstream&)>> reads;\n\n")
    file.close()
def recreate_lower_operations_file(names):
    file = open(cpp_file, 'a')
    file.write('\nint main(void){\nauto _start = std::chrono::high_resolution_clock::now();\n')
    for name in names:
        file.write('std::thread ' + name + '_thread ('+ name + '_operations);\n')
    for name in names:
        file.write(name + '_thread.join();\n')
    file.write("\nauto _stop = std::chrono::high_resolution_clock::now();\nauto _duration = std::chrono::duration_cast<std::chrono::microseconds>(_stop - _start);\nstd::fstream _file(\"OUTPUT/_results.txt\", std::fstream::app); _file<<std::endl<<\"Total execution time of all read and writes: \"<<_duration.count()<<\" microseconds\"; _file.close();\n}\n")
    file.close()
def recreate_lower_operations_table(name):
    file = open(cpp_file, 'a')
    file.write("\nstd::vector<std::stringstream>ssv(reads.size());\nstd::vector<std::thread> tv;\nfor(std::size_t i = 0; i<reads.size(); ++i)tv.emplace_back(reads[i], std::ref(ssv[i]));\nfor(auto& t : tv) t.join();\nfor(const auto& w : writes) w();\nstd::fstream file;\nmtx.lock();\nfile.open(\"OUTPUT/_results.txt\", std::fstream::app);\nfile<<\"TABLE: \"<<\""+name+"\"<<std::endl;\nfor(auto& ss : ssv) file<<ss.rdbuf()<<std::endl;\nfile.close();\nmtx.unlock();\n}\n\n")
    file.close()
def add_operation(args):
    file = open(cpp_file, 'a')
    name = args[1]
    file.write("writes.push_back([](){" + name + "_.add("+name+"{")
    i = 2
    while i < len(args):
        file.write("{"+args[i]+"},")
        i += 1
    file.write("});});\n")
    file.close()
def erase_operation(args):
    file = open(cpp_file, 'a')
    name = args[1]
    file.write("writes.push_back([](){" + name + "_.erase({" + args[2] + "});});\n")
    file.close()
def search_operation(args):
    file = open(cpp_file, 'a')
    name = args[1]
    file.write("reads.push_back([](std::stringstream& ss){"+name+" result;\nif("+name+"_.search({"+args[2]+"}, result)) ss<<result;});\n")
    file.close()
def import_csv_operation(args):
    name = args[1]
    file_name = args[2][1:-1] # <----- PROBAR
    file = open(file_name, 'r')
    lines = [line.split(',') for line in file.read().split('\n')]
    file.close()
    file = open(cpp_file, 'a')
    file.write("writes.push_back([](){")
    for line in lines:
        file.write(name+"_.add("+name+"{")
        for word in line:
            file.write("{" + word + "},")
        file.write("});\n")
    file.write("});\n")
def translate_line(args):
    if args:
        action = args[0]
        if action == "table":
            create_table(args)
        elif action == "add":
            add_operation(args)
        elif action == "erase":
            erase_operation(args)
        elif action == "search":
            search_operation(args)
        elif action == "import":
            import_csv_operation(args)

def translate(text):
    file = open("OUTPUT/_results.txt", 'w').close()
    erase_all_tables()
    recreate_upper_operations_file()
    lines = [line.split(' ') for line in text.split('\n')]
    tables = {}
    for line in lines:
        tables[line[1]] = []
    for line in lines:
        tables[line[1]].append(line)
    for name, table in tables.items():
        recreate_upper_operations_table(name)
        for line in table:
            translate_line(line)
        recreate_lower_operations_table(name)
    recreate_lower_operations_file(tables.keys())

def compile(text):
    translate(text)
    if os.system('g++ -std=c++11 -o ' + ' ' + exe_file + ' '+ cpp_file ) == 0:
        os.system(exe_file)