#include"ftp_client.h"

CFTPClient::CFTPClient():
	m_filename(""),
	m_filesize(-1),
	m_is_rest(false),
	m_file_offset(0),
{
	
}
	
CFTPClient::~CFTPClient(){
	
}

void CFTPClient::is_continue_download(){
	return m_is_rest;
}


void CFTPClient::login_server(const std::string& host){
		m_control_socket.create_socket();
	if(!m_control_socket.connect_socket(host,CONTROL_PORT){
		perror("LoginServer:control port connect error\n");
		m_control_socket.close_socket();
		return;
	}
	
	std::string response;
	int recv_ret = m_control_socket.recv_message(response);
	if(recv_ret<=0){
		perror("login_server:recv welcome message error or disconnect from server\n");
		return;
	}
	else{
		std::cout<<response<<std::endl;
	}
}

void CFTPClient::input_username(const std::string& username){
	std::string control = parse_command(FTP_COMMAND_USERNAME,username);
	send_recv_message(control);
	

}

void CFTPClient::input_password(const std::string& password){
	std::string control = parse_command(FTP_COMMAND_PASSWORD,password);
	send_recv_message(control);
}
void CFTPClient::quit_server(void){
	std::string control = parse_command(FTP_COMMAND_QUIT, "");
    send_recv_message(control);
	if(m_data_socket.get_fd()!=-1){
		m_data_socket.close_socket();
	}
	if(m_control_socket.get_fd()!=-1){
		m_control_socket.close_socket();
	}

}

bool CFTPClient::list_file(const std::string& dirname){
	std::string control = parse_command(FTP_COMMAND_LIST,dirname);
    send_recv_message(control);
	
	std::string response;
	m_control_socket.recv_message(response);
	
	struct winsize size;
	ioctl(STDIN_FILENO,TIOCGWINSZ, &size);
	int ter_width = size.ws_col;
	
	std::vector<std::string> file_vector;
	std::string filename;
	size_t max_filename_len = 0;
	std::string::size_type prv_idx = 0;
	std::string::size_type cur_idx = 0;
	
	while(true){
		prv_idx = cur_idx;
		cur_idx = response.find_first_of("\t",cur_idx + 1);
		if(cur_idx ==std::string::npos){
			file_vector.push_back(response.substr(prv_idx));
			break;
		}
		if(prv_idx==0){
			filename = response.substr(prv_idx,cur_idx-prv_idx);
		}else{
			filename = response.substr(prv_idx + 1, cur_idx - prv_idx - 1);
		}
		max_filename_len = std::max(max_filename_len,filename.length());
		file_vector.push_back(filename);
	}
	int every_len = max_filename_len + 1;
	std::cout<<every_len<<" "<<ter_width<<std::endl;
	int every_row_cnt = ter_width/every_len;
	int cur_row_cnt = 0;
	for(std::string name : file_vector){
		std::cout<<name;
		int len = every_len-name.length();
		while(len--)
			std::cout<<" ";
		if(++cur_row_cnt>=every_row_cnt)
			std::cout<<"\n";
		cur_row_cnt%=every_row_cnt;
	}
	std::cout<<std::endl;
	return true;
	
}
	
bool CFTPClient::download(const std::string& filename){
	std::string control = parse_command(FTP_COMMAND_SIZE, filename);
    send_command(control);
    std::string response;
    recv_response(response);

	std::stringstream oss(response);
	long long int file_size;
	oss>>file_size;
	if(file_size<0){
		std::cout<<"no such file"<<std::endl;
		return false;
	}
	
	control = parse_command(FTP_COMMAND_RETR,filename);
	if(send_recv_message(control)==false)
		return false;
	
	m_filename = filename;
	m_filesize = file_size;
	
	pthread_t tid;
	pthread_create(&tid,NULL,process_download,static_case<void*>(this));
	pthread_detach(tid);
	
	return true;
}

void* CFTPClient::process_download(void* arg){
	std::cout<<"in process_download"<<std::endl;
	CFTPClient* ftp_client = static_cast<CFTPClient*>(arg);
	std::string filename = ftp_client->m_filename;
	off_t file_size = ftp_client->m_filesize;
	
	char buffer[FTP_DEFAULT_BUFFER];
	bzero(buffer,FTP_DEFAULT_BUFFER);
	
	std::ofstream out;
	if(ftp_client->is_continue_download()){
		out.open(filename.c_str(),std::ios_base::binary|std::ios_base::out|std::ios_base::app);
	}else{
		out.open(filename.c_str(),std::ios_base::binary|std::ios_base::out);
	}
	if(!out.is_open){
		std::cout<<"can't open file:"<<filename<<srd::endl;
		ftp_client->m_is_rest = false;
		pthread_exit(NULL);
	}
	
	long long int recv_size = ftp_client->m_file_offset;
	std::string message;
	while(true){
		int n = ftp_client->m_data_socket.recv_message(message);
		if(n<0){
			std::cout<<"recv error"<<std::endl;
			break;
		}
		else if(n==0){
			std::cout<<"disconnect from server"<<std::endl;
			ftp_client->m_data_socket.close();
			break;
		}else{
			out.write(message.c_str(),n);
			recv_size += n;
			if(recv_size>=file_size)
				break;
		}
	}
	out.close();
	
	std::cout<<"download over"<<std::endl;
	ftp_client->m_is_rest = false;
	pthread_exit(NULL);
}


bool CFTPClient::store(const std::string& filename){
	struct stat statinfo;
	if(lstat(filename.c_str(),&statinfo)<0){
		std::cout<<"fail to store file,lsata error"<<std::endl;
		return false;
	}
	std::stringstream oss;
	oss<<filename<<"<"<<statinfo.st_size<<">";
	std::string control = parse_command(FTP_COMMAND_STOR,oss.c_str());
	if(send_recv_message(control)==false)
		return false;
	
	int filefd = open(filename.c_str(),O_RDONLY);
	sendfile(m_data_socket.get_fd(),filefd,NULL,statinfo.st_size);
	close(filefd);
	
	std::cout<<"store file over"<<std::endl;
	
	return true;
}



	bool continue_download(const std::string& offset);
	
bool CFTPClient::print_work_directory(){
	std::string control = parse_command(FTP_COMMAND_PWD,"");
	return send_recv_message(control);
}


bool CFTPClient::change_work_directory(const std::string& dirname){
	std::string control = parse_command(FTP_COMMAND_CWD,dirname);
	return send_recv_message(control);
}


bool CFTPClient::get_filesize(const std::string& filename){
	std::string control = parse_command(FTP_COMMAND_CWD,filename);
	return send_recv_message(control);
}

bool CFTPClient::send_recv_message(const std::string& control){
	if(send_command(control)==false){
		std::cerr<<"send command to server error"<<std::endl;
		return false;
	}
	
	std::string response;
	if(recv_response(response)==false){
		return false;
	}
	std::cout<<response<<std::endl;
	return true;
}

bool CFTPClient::send_command(const std::string& command){
	if(m_control_socket.sned_message(command)>0)
		return true;
	else
		return false;
}

bool CFTPClient::recv_response(std::string& response){
	if(m_control_socket.recv_message(response)>0)
		return true;
	else
		return false;
}

std::string CFTPClient::parse_command(int control,const std::string& argument){
	std::string command("");
	
	switch(control){
		case FTP_COMMAND_USERNAME:
			command = "USER " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "PASS " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "LIST " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "PWD\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "CWD " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "SIZE " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "RETR " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "STOR " + argument + "\r\n";
			break;
		case FTP_COMMAND_PASSWORD:
			command = "REST " + argument + "\r\n";
			break;
		case FTP_COMMAND_QUIT:
			command = "QUIT\r\n";
			break;
		default:
			break;
    }
    return command;
}



	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	


	
