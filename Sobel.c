
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <inttypes.h>
#define Size 4096
#define Num 470
#define Hi 310
#define Byte 1
#define LenNum 8
#define Dec 10
struct Pixel
{
    __uint8_t Red;
    __uint8_t Green;
    __uint8_t Blue;
};
struct FuncArg
{
    int From;
    int To;
    int Length;
    struct Pixel* Matrix;
    struct Pixel* Matrix_out;
};
int StrToLong(char* Name,int n)
{
 int Summ=0;
 for(int i=0;i<n;i++)
    {
        Summ*=Dec;
        Summ+=Name[i]-'0'; 
    }
 return Summ;
}

void MyCat(char* place,char* Name,int n,char* Name2,int n2)
{
 char buff[n+n2+1];
 for(int j=0;j<n+n2+1;j++) buff[j]='/';
 for(int i=0;i<n;i++)
    {
        buff[i]=Name[i];
    }
 for(int i=0;i<n2;i++)
 {
     buff[n+i]=Name2[i];
 }
 buff[n+n2]='\0';
 for(int j=0;j<n+n2+1;j++) place[j]=buff[j];
 return;
}

int* MatrVec(__int16_t* Matr,int len,int* Vec)
{
    int* Vector=(int*)malloc(len*sizeof(int));
    for(int i=0;i<len;i++)
     for(int j=0;j<len;j++)
     {
         Vector[i]+=Matr[i*len+j]*Vec[j];
     }
    return Vector;
}

int** MatrMatr(__int16_t* Matr,int len,__int16_t* Vec)
{
    int** Vector=(int**)malloc(len*sizeof(int*));
    for(int i=0;i<len;i++) Vector[i]=(int*)malloc(len*sizeof(int));
    for(int i=0;i<len;i++)
      for(int j=0;j<len;j++)
         {  
             for(int k=0;k<len;k++)
                {
                    Vector[i][j]+=Matr[i*len+k]*Vec[j+len*k];
                }
         }
    return Vector;
}

void write_num(int out,int num) {
    int buff=num;
    char num_buff[Dec]="";//define 10 
    num_buff[9]='\0';
    int p=1;
    while(buff>0)
    {

        num_buff[9-p]=buff%Dec+'0';
        buff=buff/Dec;
        p++;
    }
    write(out,(num_buff+Dec-p),p-1);
}

int MatrOther(__int16_t* Matr,int len,__int16_t* Vec)
{
    int Summ=0;
    for(int i=0;i<len;i++)
      for(int j=0;j<len;j++)
         Summ+=Matr[i*len+j]*Vec[i*len+j];
    return Summ;
}

int Double(int p)
{
    return p*p;
}


void Sobel(struct Pixel* pix,int i,int j,int len,struct Pixel* Out)
{
   __int16_t MatrX[3][3]={{-1,0,1},
                         {-2,0,2},
                         {-1,0,1}};
   __int16_t MatrY[3][3]={{-1,-2,-1},
                            {0,0,0},
                            {1,2,1}}; 
   struct Pixel Vec_first[3][3]={{pix[len*(i-1)+j-1],pix[len*(i-1)+j],pix[len*(i-1)+j+1]},
                                {pix[len*i+j-1],pix[len*i+j],pix[len*i+j+1]},
                                {pix[len*(i+1)+j-1],pix[len*(i+1)+j],pix[len*(i+1)+j+1]}};
   __int16_t buff[3][3];
   int  Vec_f[3];
   int  Vec_s[3];
   for(int p=0;p<3;p++)
   {
    for(int i=0;i<3;i++)
        {
         for(int j=0;j<3;j++)
         {
            if(p==0)
            buff[i][j]=Vec_first[i][j].Red;
            else if(p==1)
            buff[i][j]=Vec_first[i][j].Green;
            else buff[i][j]=Vec_first[i][j].Blue;
         }
        }     
     Vec_f[p]=Double(MatrOther(*MatrX,3,*buff));
     Vec_s[p]=Double(MatrOther(*MatrY,3,*buff));
     Vec_f[p]=sqrt(Vec_f[p]+Vec_s[p]);
   }
   Out[len*i+j].Red=Vec_f[0];
   Out[len*i+j].Green=Vec_f[1];
   Out[len*i+j].Blue=Vec_f[2];
   struct Pixel Pix_change;
}

void* SobelCircle(void* args)//(int FromLine,int ToLine,int ColumCount,struct Pixel* Matrix,struct Pixel*Matr_Out)
{
    struct FuncArg* Buff;
    Buff=((struct FuncArg*)args);
    for(int i=1+Buff->From;i< Buff->To-1;i++)
        for(int j=1;j<Buff->Length-1;j++)
         Sobel(Buff->Matrix,i,j,Buff->Length,Buff->Matrix_out);
    pthread_exit(NULL);
}

struct Pixel** ReadInfo(char* filename,int* lenth_a,int* lenth_b)
{
    char help[LenNum];
    int length=0;
    int useless=0;
    char Buff[Size];
    struct Pixel** Matrix;
    int file=open(filename,O_RDONLY);
    if(file<0)
    {
        printf("Can't open file");
        return NULL;
    }
    read(file,Buff,sizeof(Buff));
    int j=3;
    for(;j<100;j++)
    {

        if(Buff[j]=='#')
        {
            useless=1;
        }
        else if((Buff[j]=='\n')&&(useless))
        {
            useless=0;
            continue;
        }
        else if(useless) continue;
        else if(!*lenth_b)
        {
            if((Buff[j]>'9')||(Buff[j]<'0'))
            {
                *lenth_b=StrToLong(help,strlen(help));
                length=0;
                continue;
            }
            else 
            {       
                help[length]=Buff[j];
                length++;
                continue;
            }
        }
        else if(!*lenth_a)
        {
            if(Buff[j]=='\n')
            {
                *lenth_a=StrToLong(help,strlen(help));
                useless=1;
                break;
            }
            else 
            {
                help[length]=Buff[j];
                length++;
                continue;
            }
        }
    }
    j+=5;
    Matrix=(struct Pixel**)malloc((*lenth_a)*sizeof(struct Pixel*));
    for(int i=0;i<*lenth_a;i++)  Matrix[i]=(struct Pixel*)malloc((*lenth_b)*sizeof(struct Pixel));
    int offset_i=0;
    int offset_j=0;
    length=0;
    int R=1;
    int G=0;
    int B=0;
    lseek(file,j,SEEK_SET);
    int nread;
    unsigned long ty;
    while((nread=read(file,Buff,sizeof(Buff)))>0)
    for(j=0;j<nread;j++)
    {
        ty++;
        if(offset_j==*lenth_b)
        {
            offset_i++;
            offset_j=0;
        }
        if(R)
        {
            R--;
            Matrix[offset_i][offset_j].Red=Buff[j];
            G=1;
            continue;
        }
        if(G)
        {
            G--;
            Matrix[offset_i][offset_j].Green=Buff[j];
            B=1;
            continue;
        }
        if(B)
        {
            B--;
            Matrix[offset_i][offset_j].Blue=Buff[j];
            R=1;
            offset_j++;
            continue;
        }
    }
    return Matrix;
}
void PlaceInfo(char* string,int lenth_a,int lenth_b,struct Pixel** Matr_Out)
{
    int offset_i;
    char* placehelp=strrchr(string,'/');
    char Place[Size];
    MyCat(Place,string,strlen(string)-strlen(placehelp),"",0);
    MyCat(Place,Place,strlen(Place),"/out.pnm",strlen("/out.pnm"));
    int out=open(Place,O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
    write(out,"P6\n",strlen("P6\n"));
    write_num(out,lenth_b-2);
    write(out," ",1);
    write_num(out,lenth_a-2);
    write(out,"\n255\n",strlen("\n255\n"));
    offset_i=lseek(out,0,SEEK_CUR);
    close(out);
    FILE* fil=fopen(Place,"a+");
    fseek(fil,offset_i,SEEK_SET);
    for(int i=1;i<lenth_a-1;i++)
    {
        for(int j=1;j<lenth_b-1;j++)
        {
            for(int p=0;p<3;p++)
            {
                char k;
                if(p==0)
                {
                    k=Matr_Out[i][j].Red;
                    fputc(k,fil);
                }
                else if(p==1)
                {
                    k=Matr_Out[i][j].Green;
                    fputc(k,fil);
                }
                else 
                {
                    k=Matr_Out[i][j].Blue;
                     fputc(k,fil);
                }
            }
        }
    }
    fclose(fil);
}
int main(int argc,char* args[]) 
{
    int k=StrToLong(args[2],strlen(args[2]));//threads
    int lenth_a=0;// rows
    int lenth_b=0;//colomns
    struct Pixel** Matrix;
    struct Pixel Pix;
    Matrix=ReadInfo(args[1],&lenth_a,&lenth_b);
    struct Pixel** Matr_Out=(struct Pixel**)malloc((lenth_a)*sizeof(struct Pixel*));
    for(int i=0;i<lenth_a;i++)  Matr_Out[i]=(struct Pixel*)malloc((lenth_b)*sizeof(struct Pixel));
    int res=0;
    pthread_t* athread=( pthread_t* )malloc(k*sizeof(pthread_t));
    struct FuncArg* Buffer=(struct FuncArg*)malloc(k*sizeof(struct FuncArg));
    void* arg;
    void* thread_result;
    time_t my_time=clock();
    for(int i=0;i<k;i++)
    {
        int Step=lenth_a/k;
        Buffer[i].Length=lenth_b;
        Buffer[i].Matrix=*Matrix;
        Buffer[i].Matrix_out=*Matr_Out;
        if(!i)
        Buffer[i].From=Step*i;
        else  Buffer[i].From=Step*i-1;
        if(i!=k-1)Buffer[i].To=Step*(i+1)+1;
        else Buffer[i].To=Step*(i+1);
        arg=&Buffer[i];
        res=pthread_create(&athread[i],NULL,SobelCircle,arg);
    }
    for(int i=0;i<k;i++)
    {
        res = pthread_join(athread[i], &thread_result);
    }
    printf("%li", clock()-my_time);
    PlaceInfo(args[1],lenth_a,lenth_b,Matr_Out);
}
