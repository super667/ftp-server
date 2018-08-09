#include"ftp_client.h"
#include<iostream>
#include<sstream>

int main(int argc, char *argv[]){
	if(argc!=2){
		std::cout<<"Enter ip address"<<endl;
		return 0;
	}
	
	CFTPClient ftp；
	ftp.login_server(argv[1]);
	std::cout<<"connect successfully\n";
	std::string command;
	std::string argument;
	
	std::stringstream oss;
	while(true){
		command.clear();
		argument.clear();
		getline(std::cin,command);
		oss.str("");
		oss.clear();
		oss<<command;
		oss>>command>>argument;
		
		if(command=="USER"){
			ftp.input_username(argument);
		}else if(command=="PASS"){
			ftp.input_password(argument);
		}else if(command=="LIST"){
			ftp.list_file(argument);
		}else if(command == "PWD"){
            ftp.print_work_directory();
        }else if(command == "CWD"){
            ftp.change_work_directory(argument);
        }else if(command == "QUIT"){
            ftp.quit_server();
            break;
        }else if(command == "RETR"){
            ftp.download(argument);
        }else if(command == "STOR"){
            ftp.store(argument);
        }else if(command == "SIZE"){
            ftp.get_filesize(argument);
		}
	}
	return 0;
}