#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <process.h>
#include <fstream>
#include <errno.h>
#include <string.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;
#define  PORT  8086        //�����׽��ֵ�ȱʡЭ��˿ں�
#define	 QUEL  10          //�����׽��ֵ�������д�С
#define  MAX   4096
bool mark=0;//1:�Ѿ��ﵽ10��ͼƬ
bool pend=0;//1:��ȡ��1��ͼƬ
int  dirname = 1;//�ļ��б��
int  quence1 = 1;//�ļ���1��ͼƬ�洢����
int  quence2 = 1;//
int  quence3 = 1;//
char EOF0=255;//ͼƬ������ʶ��
char EOF1=217;
char recvBuf[MAX]={0};//���ջ���
char p[MAX];//����ļ�·��
void Recv(SOCKET sockClient,int dirname, int& quence);
int main(int argc, char* argv[])
{
    // ����socket��̬���ӿ�(dll)
	WORD wVersionRequested;
	WSADATA wsaData;
	char last[3]={0,0,0};
	int err;
	wVersionRequested = MAKEWORD(2, 2);//�����׽��ֿ� �汾��Ϊ2.2
	//���
	err = WSAStartup(wVersionRequested, &wsaData);
	if ( err != 0)
	{
		cout << "error = " << err << endl;
		WSACleanup();
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
	    // ����Ƿ�Ϊ2.2�汾
		// ����Ļ�������WSACleanup()�����Ϣ����������
		WSACleanup();
		return -1;
	}

	// ����������socket��������ʽ�׽��֣������׽��ֺ�sockSrv
	// SOCKET socket(int af, int type, int protocol);
	// ��һ��������ָ����ַ�� TCP/IPֻ����AF_INET
	// �ڶ�����ѡ���׽��ֵ�����(��ʽ�׽���)�����������ض���ַ�������Э�飨0Ϊ�Զ���
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if(sockSrv == INVALID_SOCKET)
    {
        cout<<"fail to creat socket!"<<endl;
        exit(1);
    }
	sockaddr_in addSrv; //���屾����ַ��Ϣ
	addSrv.sin_family = AF_INET;
	addSrv.sin_addr.S_un.S_addr = inet_addr("192.168.137.1");// ���ػ�·��ַ��127.0.0.1;htonl(INADDR_ANY);//�Զ���䱾��IP
	int port;
	if(argc>1)
        port = atoi(argv[1]);//���ָ���˶˿ںţ�����ת��Ϊ����
    else
        port = PORT;
    if(port>0)//���Զ˿ں��Ƿ�Ϸ�
        addSrv.sin_port = htons((short)port);//�˿ں�ת��Ϊ�����ֽ�˳��
    else
    {
        printf("bad port number %s\n",argv[1]);
    }
    // �׽���sockSrv�뱾�ص�ַ����
    // int bind(SOCKET s, const struct sockaddr* name, int namelen);
	// ��һ��������ָ����Ҫ�󶨵��׽��֣�
	// �ڶ���������ָ�����׽��ֵı��ص�ַ��Ϣ���õ�ַ�ṹ�������õ�����Э��Ĳ�ͬ����ͬ
	// ������������ָ��������Э���ַ�ĳ���
	err = bind(sockSrv, (SOCKADDR*)&addSrv, sizeof(addSrv));// �ڶ�����Ҫǿ������ת��
    if(err == SOCKET_ERROR)
    {
        cout<<"fail to bind!"<<endl;
        exit(1);
    }

	// ����Ϊ����ģʽ����������
	// int listen(SOCKET s,  int backlog);void Recv(SOCKET sockClient)
	// ��һ������ָ����Ҫ���õ��׽��֣��ڶ�������Ϊ���ȴ����Ӷ��е���󳤶ȣ�
	err = listen(sockSrv, QUEL);
    if(err == SOCKET_ERROR)
    {
        cout<<"fail to listen!"<<endl;
        exit(1);
    }
	sockaddr_in addrClient;//�ͻ��˵�ַ��Ϣ
	int len = sizeof(SOCKADDR);
    cout<<"�ȴ�������..."<<endl;
    while(1)
    {
        SOCKET sockClient = accept(sockSrv, (SOCKADDR*)&addrClient, &len);//�����µ��׽����뵱ǰ�ͻ�������

        if(sockClient == SOCKET_ERROR)
            cout<<"���մ���"<<endl;
        else
        {
            printf("���յ�һ�����ӣ�%s", inet_ntoa(addrClient.sin_addr));
            cout<<"------------------------------------------------------\n";
            char recvBuf[MAX]={0};
            char p[MAX];
            int  byte = 0;
            char  quence ='0';
            while(1)
            {
                switch(dirname)
                {
                    case 1: Recv(sockClient,dirname, quence1);
                            break;
                    case 2: Recv(sockClient,dirname, quence2);
                            break;
                    case 3: Recv(sockClient,dirname, quence3);
                            break;
                }
                dirname++;
                if(dirname==4)
                  dirname=1;
                Sleep(1000);
            }
            closesocket(sockClient);
            cout<<"�Ѿ��Ͽ��Ϳͻ��˵�����"<<endl;
            Sleep(1000);
            cout<<"------------------------------------------------------\n";
        }
    }
    closesocket(sockSrv);
    WSACleanup();// ��ֹ���׽��ֿ��ʹ��
	system("pause");

	return 0;
}
//���ܲ��洢һ��ͼƬ
// sockClient���ͻ����׽��ֱ��
// fname���ļ��б��
void Recv(SOCKET sockClient,int dirname, int& quence)
{
    int  byte = 0;
    sprintf(p,"save_images\\t%d\\%d.jpg",dirname,quence);
    byte = recv(sockClient, recvBuf, sizeof(recvBuf),0);//��������
    if(byte > 0)
    {
        cout<<"��ʼ��������"<<endl;
        cout<<"TCP�������˽��յ����ֽ�����"<<endl;
        ofstream output( p, ios::out | ios::binary );
        if( output.fail() )
        {
            printf("%s", strerror(errno));
            cout << "Open output file error!" << endl;
            exit( -1 );
        }
        for(int i=0; i<byte; i++)
        {
            output.write ((char *) &recvBuf[i], sizeof(char) );
            //printf("%c",recvBuf[i]);
        }
            while(!pend)
        {
            byte = recv(sockClient, recvBuf, sizeof(recvBuf),0);//��������
            printf("%d\n",byte);
            if(byte <= 0)
            continue;
            //recvBuf[byte-1];
            if((recvBuf[byte-4]==EOF0)&&(recvBuf[byte-3]==EOF1)&&(recvBuf[byte-2]==EOF0)&&(recvBuf[byte-1]==EOF1-1))
            {
                for(int i=0; i<byte-4; i++)
                {
                    output.write ((char *) &recvBuf[i], sizeof(char) );
                    //printf("%c",recvBuf[i]);
                }
                cout<<"�������"<<endl;
                pend=1;
                quence++;
                if(quence==11)
                    quence=1;
            }
            else
            {
                for(int i=0; i<byte; i++)
                {
                    output.write ((char *) &recvBuf[i], sizeof(char) );
                    //printf("%c",recvBuf[i]);
                }
            }
            memset(recvBuf,'\0', MAX);
        }
        output.close();
        pend=0;
    }
    //sprintf(p,"C:\\Users\\wwz\\Desktop\\Server2\\server\\bin\\Debug\\save_images\\t%d\\%d.jpg",dirname,quence);


}

