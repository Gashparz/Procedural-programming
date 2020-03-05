#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixeli;
typedef struct
{
    unsigned int lungime;
    unsigned int latime;
    unsigned int poz;
    float media;
    float deviatia;
} ferestre;
uint32_t xorshift32(uint32_t state[])
{
    uint32_t x = state[0];
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state[0] = x;
    return x;
}
int Date(char *FileNameIn,unsigned int *w,unsigned int *h,int *dim)
{
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului pentru extragerea datelor!\n");
        return -1;
    }
    fseek(fin,2,SEEK_SET);
    fread(dim,sizeof(char),4,fin);
    fseek(fin,18,SEEK_SET);
    fread(w,sizeof(char),4,fin);
    fread(h,sizeof(char),4,fin);
    fclose(fin);
}
int pixel(char *FileNameIn,pixeli *rgb,unsigned int w,unsigned int h)
{
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului pentru a extrage pixelii!\n");
        return -1;
    }
    fseek(fin,54,SEEK_SET);
    for(int i=0; i<w*h; i++)
    {
        fread(&rgb[i].b,sizeof(char),1,fin);
        fread(&rgb[i].g,sizeof(char),1,fin);
        fread(&rgb[i].r,sizeof(char),1,fin);
    }
}
int liniarizare(char *FileNameIn,unsigned char *lin,unsigned int w,unsigned int h)
{
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului pentru liniarizare!\n");
        return -1;
    }
    fseek(fin,54,SEEK_SET);
    for(int i=0; i<(3*w*h); i++)
        fread(&lin[i],sizeof(char),1,fin);
    fclose(fin);
}
int liniarizare_ext(char *FileNameIn,char *FileNameOut,unsigned char *lin, unsigned int w,unsigned int h)
{
    unsigned char aux;
    int padding=0;
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    FILE* fout;
    fout=fopen(FileNameOut,"wb");
    if(fin==NULL||fout==NULL)
    {
        printf("Eroare la deschiderea fisierelor pentru liniarizarea exterioara!\n");
        return -1;
    }
    if(w % 4 != 0)
        padding=4-(3*w) % 4;
    else
        padding=0;
    if(padding!=0)
        w=w*3+padding;
    fseek(fin,0,SEEK_SET);
    for(int i=0; i<54; i++)
    {
        fread(&aux,sizeof(char),1,fin);
        fwrite(&aux,sizeof(char),1,fout);
    }
    fflush(fin);
    fseek(fin,54,SEEK_SET);
    for(int j=0; j<(3*w*h); j++)
    {
        fwrite(&lin[j],sizeof(char),1,fout);
    }
    fclose(fin);
    fclose(fout);
}
void durstenfeld(uint32_t *R,unsigned int w,unsigned int h,uint32_t *R2)
{
    unsigned int k=1,cnt=0;
    unsigned int aux=0;
    for(int i=(w*h)-1; i>=0; i--)
    {
        aux=R[k]%(i+1);
        R2[aux]=R2[aux]^R2[i];
        R2[i]=R2[aux]^R2[i];
        R2[aux]=R2[i]^R2[aux];
        k++;
    }
}
int criptare(char *FileNameIn,char *SecreteKey,char *FileCript,unsigned char *lin,unsigned char *lin2,unsigned int w,unsigned int h,unsigned char *cip)
{
    unsigned char *bytes=malloc(4*sizeof(unsigned char));
    unsigned char *bytesSV=malloc(4*sizeof(unsigned char));
    uint32_t *R=malloc(((2*w*h)-1)*sizeof(uint32_t));
    uint32_t *R2=malloc(((w*h))*sizeof(uint32_t));
    uint32_t SV=0;
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    FILE* fout;
    fout=fopen(FileCript,"wb");
    FILE* skey;
    skey=fopen(SecreteKey,"r");
    if(fin==NULL||fout==NULL||skey==NULL)
    {
        printf("Eroare la deschiderea fisierelor pentru criptare!\n");
        return -1;
    }
    fscanf(skey,"%u",&R[0]);
    fscanf(skey,"%u",&SV);
    uint32_t state[1]= {R[0]};
    for(int i=1; i<=((2*w*h)-1); i++)
    {
        R[i]=xorshift32(state);
    }
    for(int i=0; i<(w*h); i++)
    {
        R2[i]=i;
    }
    durstenfeld(R,w,h,R2);
    for(int i=0; i<3*w*h; i++)
        lin2[i]=lin[i];
    for(int i=0; i<w*h; i++)
    {
        lin2[3*R2[i]]=lin[3*i];
        lin2[3*R2[i]+1]=lin[3*i+1];
        lin2[3*R2[i]+2]=lin[3*i+2];
    }
    bytes[0]=(R[w*h]&0xFF);
    bytes[1]=((R[w*h]>>8)&0xFF);
    bytes[2]=((R[w*h]>>16)&0xFF);
    bytes[3]=((R[w*h]>>24)&0xFF);
    bytesSV[0]=(SV&0xFF);
    bytesSV[1]=((SV>>8)&0xFF);
    bytesSV[2]=((SV>>16)&0xFF);
    cip[0]=bytesSV[2]^lin2[0]^bytes[2];
    cip[1]=bytesSV[1]^lin2[1]^bytes[1];
    cip[2]=bytesSV[0]^lin2[2]^bytes[0];
    for(int i=1; i<=w*h-1; i++)
    {
        bytes[0]=(R[w*h+i]&0xFF);
        bytes[1]=((R[w*h+i]>>8)&0xFF);
        bytes[2]=((R[w*h+i]>>16)&0xFF);
        cip[3*i]=cip[3*i-1]^lin2[3*i]^bytes[2];
        cip[3*i+1]=cip[3*i-2]^lin2[3*i+1]^bytes[1];
        cip[3*i+2]=cip[3*i-3]^lin2[3*i+2]^bytes[0];
    }
    liniarizare_ext(FileNameIn,FileCript,cip,w,h);
    free(bytes);
    free(bytesSV);
    free(R);
    free(R2);
    fclose(fin);
    fclose(fout);
    fclose(skey);
}
int decriptare(char* FileNameIn,char* FileCript,char* SecreteKey,char* FileDeCript,unsigned char *cip,unsigned char *cipd,unsigned int w,unsigned int h,unsigned char *D)
{
    uint32_t *R=malloc(((2*w*h)-1)*sizeof(uint32_t));
    uint32_t *R2=malloc(((w*h))*sizeof(uint32_t));
    uint32_t *R3=malloc(((w*h))*sizeof(uint32_t));
    unsigned char *bytes=malloc(4*sizeof(unsigned char));
    unsigned char *bytesSV=malloc(4*sizeof(unsigned char));
    uint32_t SV=0;
    FILE* fc;
    fc=fopen(FileCript,"rb");
    FILE* fi;
    fi=fopen(FileNameIn,"rb");
    FILE* skey;
    skey=fopen(SecreteKey,"r");
    if(fc==NULL||fi==NULL||skey==NULL)
    {
        printf("Eroare la deschiderea fisierelor pentru decriptare!\n");
        return -1;
    }
    fscanf(skey,"%u",&R[0]);
    fscanf(skey,"%u",&SV);
    uint32_t state[1]= {R[0]};
    for(int i=1; i<=((2*w*h)-1); i++)
    {
        R[i]=xorshift32(state);
    }
    for(int i=0; i<w*h; i++)
    {
        R2[i]=i;
    }
    durstenfeld(R,w,h,R2);
    for(int i=0; i<w*h; i++)
        R3[R2[i]]=i;
    bytes[0]=(R[w*h]&0xFF);
    bytes[1]=((R[w*h]>>8)&0xFF);
    bytes[2]=((R[w*h]>>16)&0xFF);
    bytes[3]=((R[w*h]>>24)&0xFF);
    bytesSV[0]=(SV&0xFF);
    bytesSV[1]=((SV>>8)&0xFF);
    bytesSV[2]=((SV>>16)&0xFF);
    cipd[0]=bytesSV[2]^cip[0]^bytes[2];
    cipd[1]=bytesSV[1]^cip[1]^bytes[1];
    cipd[2]=bytesSV[0]^cip[2]^bytes[0];
    for(int i=1; i<=w*h-1; i++)
    {
        bytes[0]=(R[w*h+i]&0xFF);
        bytes[1]=((R[w*h+i]>>8)&0xFF);
        bytes[2]=((R[w*h+i]>>16)&0xFF);
        cipd[3*i]=cip[3*i-1]^cip[3*i]^bytes[2];
        cipd[3*i+1]=cip[3*i-2]^cip[3*i+1]^bytes[1];
        cipd[3*i+2]=cip[3*i-3]^cip[3*i+2]^bytes[0];
    }
    for(int i=0; i<w*h; i++)
    {
        D[3*R3[i]]=cipd[3*i];
        D[3*R3[i]+1]=cipd[3*i+1];
        D[3*R3[i]+2]=cipd[3*i+2];
    }
    liniarizare_ext(FileNameIn,FileDeCript,D,w,h);
    free(R);
    free(R2);
    free(R3);
    free(bytes);
    free(bytesSV);
    fclose(fc);
    fclose(fi);
    fclose(skey);
}
int chipatrat_r(char* FileNameIn,unsigned int w,unsigned int h)
{
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
        printf("Eroare!");
    pixeli *rgb=malloc((w*h)*sizeof(pixeli));
    pixel(FileNameIn,rgb,w,h);
    int *f_r=malloc(256*sizeof(int));
    for(int i=0; i<256; i++)
        f_r[i]=0;
    unsigned char *v=malloc(256*sizeof(unsigned char));
    for(int i=0; i<256; i++)
        v[i]=i;

    for(int i=0; i<256; i++)
        for(int j=0; j<w*h; j++)
        {
            if(rgb[j].r==v[i])
                f_r[i]++;
        }
    double fm_r=(w*h)/256;
    double chi_r=0;
    for(int i=0; i<=255; i++)
    {
        chi_r=chi_r+(((f_r[i]-fm_r)*(f_r[i]-fm_r))/fm_r) ;
    }
    printf("%f ",chi_r);
    free(v);
    free(f_r);
    free(rgb);
}
int chipatrat_g(char* FileNameIn,unsigned int w,unsigned int h)
{
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
        printf("Eroare!");
    pixeli *rgb=malloc((w*h)*sizeof(pixeli));
    pixel(FileNameIn,rgb,w,h);
    int *f_g=malloc(256*sizeof(int));
    for(int i=0; i<256; i++)
        f_g[i]=0;
    unsigned char *v=malloc(256*sizeof(unsigned char));
    for(int i=0; i<256; i++)
        v[i]=i;

    for(int i=0; i<256; i++)
        for(int j=0; j<w*h; j++)
        {
            if(rgb[j].g==v[i])
                f_g[i]++;
        }
    double fm_g=(w*h)/256;
    double chi_g=0;
    for(int i=0; i<=255; i++)
    {
        chi_g=chi_g+(((f_g[i]-fm_g)*(f_g[i]-fm_g))/fm_g) ;
    }
    printf("%f ",chi_g);
    free(f_g);
    free(v);
    free(rgb);
}
int chipatrat_b(char* FileNameIn,unsigned int w,unsigned int h)
{

    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
        printf("Eroare!");
    pixeli *rgb=malloc((w*h)*sizeof(pixeli));
    pixel(FileNameIn,rgb,w,h);
    int *f_b=malloc(256*sizeof(int));
    for(int i=0; i<256; i++)
        f_b[i]=0;
    unsigned char *v=malloc(256*sizeof(unsigned char));
    for(int i=0; i<256; i++)
        v[i]=i;

    for(int i=0; i<256; i++)
        for(int j=0; j<w*h; j++)
        {
            if(rgb[j].b==v[i])
                f_b[i]++;
        }
    double fm_b=(w*h)/256;
    double chi_b=0;
    for(int i=0; i<=255; i++)
    {
        chi_b=chi_b+(((f_b[i]-fm_b)*(f_b[i]-fm_b))/fm_b) ;
    }
    printf("%f ",chi_b);
    free(f_b);
    free(v);
    free(rgb);
}
int ChiPatrat(char* FileNameIn,char* FileCript,unsigned int w,unsigned int h)
{
    printf("Valorile testului chi pentru imaginea initiala sunt: ");
    chipatrat_r(FileNameIn,w,h);
    chipatrat_g(FileNameIn,w,h);
    chipatrat_b(FileNameIn,w,h);
    printf("\nValorile testului chi pentru imaginea criptata sunt: ");
    chipatrat_r(FileCript,w,h);
    chipatrat_g(FileCript,w,h);
    chipatrat_b(FileCript,w,h);
}
int pixel2(char *FileNameIn,pixeli **rgb,unsigned int sablon_w,unsigned int sablon_h)
{
    FILE* fin;
    fin=fopen(FileNameIn,"rb");
    if(fin==NULL)
    {
        printf("Eroare la deschiderea fisierului pentru a extrage pixelii!\n");
        return -1;
    }
    fseek(fin,54,SEEK_SET);
    for(int i=0; i<sablon_w; i++)
        for(int j=0; j<sablon_h; j++)
        {
            fread(&rgb[i][j].b,sizeof(char),1,fin);
            fread(&rgb[i][j].g,sizeof(char),1,fin);
            fread(&rgb[i][j].r,sizeof(char),1,fin);
        }
}
void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
    FILE *fin, *fout;
    unsigned int dim_img, latime_img, inaltime_img;
    unsigned char pRGB[3], header[54], aux;
    printf("nume_fisier_sursa = %s \n",nume_fisier_sursa);
    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("nu am gasit imaginea sursa din care citesc");
        return;
    }
    fout = fopen(nume_fisier_destinatie, "wb+");
    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in octeti: %u\n", dim_img);
    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);
    fseek(fin,0,SEEK_SET);
    unsigned char c;
    while(fread(&c,1,1,fin)==1)
    {
        fwrite(&c,1,1,fout);
        fflush(fout);
    }
    fclose(fin);
    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    printf("padding = %d \n",padding);
    fseek(fout, 54, SEEK_SET);
    int i,j;
    for(i = 0; i < inaltime_img; i++)
    {
        for(j = 0; j < latime_img; j++)
        {
            fread(pRGB, 3, 1, fout);
            aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
            pRGB[0] = pRGB[1] = pRGB[2] = aux;
            fseek(fout, -3, SEEK_CUR);
            fwrite(pRGB, 3, 1, fout);
            fflush(fout);
        }
        fseek(fout,padding,SEEK_CUR);
    }
    fclose(fout);
}
int grey(unsigned int *sablon_w,unsigned int *sablon_h)
{
    char nume_img_sursa[] = "test.bmp";
    char nume_img_grayscale[] = "test_grayscale.bmp";
    grayscale_image(nume_img_sursa, nume_img_grayscale);
    char cifra0[]="cifra0.bmp";
    char gcifra0[]="gcifra0.bmp";
    grayscale_image(cifra0,gcifra0);
    char cifra1[]="cifra0.bmp";
    char gcifra1[]="gcifra0.bmp";
    grayscale_image(cifra1,gcifra1);
    char cifra2[]="cifra2.bmp";
    char gcifra2[]="gcifra2.bmp";
    grayscale_image(cifra2,gcifra2);
    char cifra3[]="cifra3.bmp";
    char gcifra3[]="gcifra3.bmp";
    grayscale_image(cifra3,gcifra3);
    char cifra4[]="cifra4.bmp";
    char gcifra4[]="gcifra4.bmp";
    grayscale_image(cifra4,gcifra4);
    char cifra5[]="cifra5.bmp";
    char gcifra5[]="gcifra5.bmp";
    grayscale_image(cifra5,gcifra5);
    char cifra6[]="cifra6.bmp";
    char gcifra6[]="gcifra6.bmp";
    grayscale_image(cifra6,gcifra6);
    char cifra7[]="cifra7.bmp";
    char gcifra7[]="gcifra7.bmp";
    grayscale_image(cifra7,gcifra7);
    char cifra8[]="cifra8.bmp";
    char gcifra8[]="gcifra8.bmp";
    grayscale_image(cifra8,gcifra8);
    char cifra9[]="cifra9.bmp";
    char gcifra9[]="gcifra9.bmp";
    grayscale_image(cifra9,gcifra9);
}
void pixel_all(pixeli **rgbt,pixeli **rgb0,pixeli **rgb1,pixeli **rgb2,pixeli **rgb3,pixeli **rgb4,pixeli **rgb5,pixeli **rgb6,pixeli **rgb7,pixeli **rgb8,pixeli **rgb9,unsigned int imagine_w,unsigned int imagine_h,unsigned int sablon_w,unsigned int sablon_h)
{
    char cifra0[]="cifra0.bmp";
    pixel2(cifra0,rgb0,sablon_w,sablon_h);
    char cifra1[]="cifra0.bmp";
    pixel2(cifra1,rgb1,sablon_w,sablon_h);
    char cifra2[]="cifra2.bmp";
    pixel2(cifra2,rgb2,sablon_w,sablon_h);
    char cifra3[]="cifra3.bmp";
    pixel2(cifra3,rgb3,sablon_w,sablon_h);
    char cifra4[]="cifra4.bmp";
    pixel2(cifra4,rgb4,sablon_w,sablon_h);
    char cifra5[]="cifra5.bmp";
    pixel2(cifra5,rgb5,sablon_w,sablon_h);
    char cifra6[]="cifra6.bmp";
    pixel2(cifra6,rgb6,sablon_w,sablon_h);
    char cifra7[]="cifra7.bmp";
    pixel2(cifra7,rgb7,sablon_w,sablon_h);
    char cifra8[]="cifra8.bmp";
    pixel2(cifra8,rgb8,sablon_w,sablon_h);
    char cifra9[]="cifra9.bmp";
    pixel2(cifra9,rgb9,sablon_w,sablon_h);

}
int s_mediu(pixeli **rgb0,unsigned int sablon_w,unsigned int sablon_h)
{
    float media=0;
    for(int i=0; i<sablon_w; i++)
        for(int j=0; j<sablon_h; j++)
            media=media+rgb0[i][j].r;
    media=media/165;
    return media;
}
int deviatii(pixeli **rgb0,unsigned int sablon_w,unsigned int sablon_h,float media_s)
{
    float aux=0;
    float devi=0;
    for(int i=0; i<sablon_w; i++)
        for(int j=0; j<sablon_h; j++)
            aux=aux+((rgb0[i][j].r-media_s)*(rgb0[i][j].r-media_s));
    devi=sqrt(aux/((sablon_h*sablon_w)-1));
    return devi;
}
int corelatie(ferestre *fer,float **corr,pixeli **rgbt,pixeli **rgb0,pixeli **rgb1,pixeli **rgb2,pixeli **rgb3,pixeli **rgb4,pixeli **rgb5,pixeli **rgb6,pixeli **rgb7,pixeli **rgb8,pixeli **rgb9,unsigned int sablon_w,unsigned int sablon_h,float *media_s,float *deviatii_s,unsigned int imagine_w,unsigned int imagine_h)
{
    int k=0;
    float fi=0;
    float aux=0;
    float aux2=0;
    float deviatief=0;
    media_s[0]=s_mediu(rgb0,sablon_w,sablon_h);
    media_s[1]=s_mediu(rgb1,sablon_w,sablon_h);
    media_s[2]=s_mediu(rgb2,sablon_w,sablon_h);
    media_s[3]=s_mediu(rgb3,sablon_w,sablon_h);
    media_s[4]=s_mediu(rgb4,sablon_w,sablon_h);
    media_s[5]=s_mediu(rgb5,sablon_w,sablon_h);
    media_s[6]=s_mediu(rgb6,sablon_w,sablon_h);
    media_s[7]=s_mediu(rgb7,sablon_w,sablon_h);
    media_s[8]=s_mediu(rgb8,sablon_w,sablon_h);
    media_s[9]=s_mediu(rgb9,sablon_w,sablon_h);
    deviatii_s[0]=deviatii(rgb0,sablon_w,sablon_h,media_s[0]);
    deviatii_s[1]=deviatii(rgb1,sablon_w,sablon_h,media_s[1]);
    deviatii_s[2]=deviatii(rgb2,sablon_w,sablon_h,media_s[2]);
    deviatii_s[3]=deviatii(rgb3,sablon_w,sablon_h,media_s[3]);
    deviatii_s[4]=deviatii(rgb4,sablon_w,sablon_h,media_s[4]);
    deviatii_s[5]=deviatii(rgb5,sablon_w,sablon_h,media_s[5]);
    deviatii_s[6]=deviatii(rgb6,sablon_w,sablon_h,media_s[6]);
    deviatii_s[7]=deviatii(rgb7,sablon_w,sablon_h,media_s[7]);
    deviatii_s[8]=deviatii(rgb8,sablon_w,sablon_h,media_s[8]);
    deviatii_s[9]=deviatii(rgb9,sablon_w,sablon_h,media_s[9]);
    for(int x=0; x<imagine_h-sablon_h; x++)
    {

        for(int y=0; y<imagine_w-sablon_w; y++)
        {
            for(int i=x; i<=x+sablon_h; i++)
                for(int j=y; j<=y+sablon_w; j++)
                    fi=fi+rgbt[j][i].r;
            fi=fi/165;
            aux=aux+(((rgbt[y][x].r)-fi)*((rgbt[y][x].r)-fi));
            deviatief=sqrt(aux/((sablon_h*sablon_w)-1));
            fer[x].deviatia=deviatief;
            fer[x].media=fi;
        }
        for(int i=0; i<sablon_w; i++)
            for(int j=0; j<sablon_h; j++)
                aux2=aux2+((rgbt[i][j].r-fer[k].media)*(rgb0[i][j].r-media_s[0]))/(fer[k].deviatia*deviatii_s[0]);
        corr[0][k]=aux2/165;
        k++;
    }
}
int templatematching(char* sab,char* nume_img_sursa,ferestre *fer,float **corr,pixeli **rgbt,pixeli **rgb0,pixeli **rgb1,pixeli **rgb2,pixeli **rgb3,pixeli **rgb4,pixeli **rgb5,pixeli **rgb6,pixeli **rgb7,pixeli **rgb8,pixeli **rgb9,unsigned int sablon_w,unsigned int sablon_h,float *media_s,float *deviatii_s,unsigned int imagine_w,unsigned int imagine_h)
{
    corelatie(fer,corr,rgbt,rgb0,rgb1,rgb2,rgb3,rgb4,rgb5,rgb6,rgb7,rgb8,rgb9,sablon_w,sablon_h,media_s,deviatii_s,imagine_w,imagine_h);

}
int main()
{
    char FileNameIn[101];
    char FileNameOut[101];
    char SecreteKey[101];
    char FileCript[101];
    char FileDeCript[101];
    printf("Fisierul care contine imaginea initiala: ");
    fgets(FileNameIn, 101, stdin);
    FileNameIn[strlen(FileNameIn) - 1] = '\0';
    printf("\nCalea imaginii liniarizate: ");
    fgets(FileNameOut,101, stdin);
    FileNameOut[strlen(FileNameOut)-1]='\0';
    printf("\nCheia secreta: ");
    fgets(SecreteKey,101,stdin);
    SecreteKey[strlen(SecreteKey)-1]='\0';
    printf("\nCalea imaginii criptate: ");
    fgets(FileCript,101,stdin);
    FileCript[strlen(FileCript)-1]='\0';
    printf("\nCalea imaginii decriptate");
    fgets(FileDeCript,101,stdin);
    FileDeCript[strlen(FileDeCript)-1]='\0';
    unsigned int w=0,h=0,dim=0;
    Date(FileNameIn,&w,&h,&dim);
    unsigned char *lin=malloc((3*w*h)*sizeof(unsigned char));
    liniarizare(FileNameIn,lin,w,h);
    liniarizare_ext(FileNameIn,FileNameOut,lin,w,h);
    unsigned char *lin2=malloc((3*w*h)*sizeof(unsigned char));
    unsigned char *cip=malloc((3*w*h)*sizeof(unsigned char));
    criptare(FileNameIn,SecreteKey,FileCript,lin,lin2,w,h,cip);
    unsigned char *cipd=malloc((3*w*h)*sizeof(unsigned char));
    unsigned char *D=malloc((3*w*h)*sizeof(unsigned char));
    decriptare(FileNameIn,FileCript,SecreteKey,FileDeCript,cip,cipd,w,h,D);
    ChiPatrat(FileNameIn,FileCript,w,h);
    free(lin);
    free(lin2);
    free(cip);
    free(cipd);
    free(D);
    ///Template Matching
    char nume_img_sursa[101];
    char sab[101];
    printf("Fisierul care contine imaginea initiala: ");
    fgets(nume_img_sursa, 101, stdin);
    nume_img_sursa[strlen(nume_img_sursa) - 1] = '\0';
    printf("Fisierul care contine sablonul: ");
    fgets(sab, 101, stdin);
    sab[strlen(sab) - 1] = '\0';
    unsigned int sablon_w=0,sablon_h=0,imagine_w=0,imagine_h=0,dim1,dim2;
    grey(&sablon_w,&sablon_h);
    char cifra0[]="cifra0.bmp";
    Date(cifra0,&sablon_w,&sablon_h,&dim1);
    Date(nume_img_sursa,&imagine_w,&imagine_h,&dim2);
    float *media_s=malloc(10*sizeof(float));
    float *deviatii_s=malloc(10*sizeof(float));
    pixeli **rgbt=(pixeli**)malloc((3*imagine_h*imagine_w)*sizeof(pixeli));
    for(int i=0; i<imagine_h*imagine_w; i++)
        rgbt[i]=(pixeli *)malloc((3*imagine_h*imagine_w)*sizeof(pixel));
    pixel2(nume_img_sursa,rgbt,imagine_w,imagine_h);
    pixeli **rgb0=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb0[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb1=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb1[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb2=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb2[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb3=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb3[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb4=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb4[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb5=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb5[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb6=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb6[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb7=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb7[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb8=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb8[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    pixeli **rgb9=(pixeli**)malloc((sablon_w)*sizeof(pixeli*));
    for(int i=0; i<sablon_h; i++)
        rgb9[i]=(pixeli *)malloc(sablon_h*sizeof(pixeli));
    float **corr=(float **)malloc((imagine_w*4)*sizeof(float*));
    for(int i=0; i<imagine_h*3; i++)
        corr[i]=(float*)malloc((imagine_h*3)*sizeof(float));
    ferestre *fer=malloc(sizeof(ferestre)*(3*imagine_h*imagine_w));
    pixel_all(rgbt,rgb0,rgb1,rgb2,rgb3,rgb4,rgb5,rgb6,rgb7,rgb8,rgb9,imagine_w,imagine_h,sablon_w,sablon_h);
    ///corelatie(rgbt,rgb0,rgb1,rgb2,rgb3,rgb4,rgb5,rgb6,rgb7,rgb8,rgb9,sablon_w,sablon_h,media_s,deviatii_s,imagine_w,imagine_h);
    templatematching(sab,nume_img_sursa,fer,corr,rgbt,rgb0,rgb1,rgb2,rgb3,rgb4,rgb5,rgb6,rgb7,rgb8,rgb9,sablon_w,sablon_h,media_s,deviatii_s,imagine_w,imagine_h);
    free(media_s);
    free(deviatii_s);
    for(int i=0; i<imagine_h*imagine_w; i++)
        free(rgbt[i]);
    free(rgbt);
    for(int i=0; i<sablon_h; i++)
        free(rgb0[i]);
    free(rgb0);
    for(int i=0; i<sablon_h; i++)
        free(rgb1[i]);
    free(rgb1);
    for(int i=0; i<sablon_h; i++)
        free(rgb2[i]);
    free(rgb2);
    for(int i=0; i<sablon_h; i++)
        free(rgb3[i]);
    free(rgb3);
    for(int i=0; i<sablon_h; i++)
        free(rgb4[i]);
    free(rgb4);
    for(int i=0; i<sablon_h; i++)
        free(rgb5[i]);
    free(rgb5);
    for(int i=0; i<sablon_h; i++)
        free(rgb6[i]);
    free(rgb6);
    for(int i=0; i<sablon_h; i++)
        free(rgb7[i]);
    free(rgb7);
    for(int i=0; i<sablon_h; i++)
        free(rgb8[i]);
    free(rgb8);
    for(int i=0; i<sablon_h; i++)
        free(rgb9[i]);
    free(rgb9);
    free(fer);
    for(int i=0; i<imagine_h*3; i++)
    free(corr[i]);
    free(corr);
    return 0;
}
