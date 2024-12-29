#include "decode.h"
#include "common.h"
#include<stdlib.h>








Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2],".bmp")!=NULL)
    {
        decInfo->src_img_name=argv[2];
    }
    else{
        printf("ERROR: For decoding Argument 3 should be a .bmp file!\n");
        return e_failure;
    }
    if(argv[3]!=NULL)
    {
        decInfo->dest_file_name=strtok(argv[3],".");
        
    }
    else{
        decInfo->dest_file_name="output";
    }
    //printf("%s\n",decInfo->dest_file_name);
    return e_success;
}

Status open_files_decoding(DecodeInfo *decInfo)
{
    decInfo->fptr_src_image=fopen(decInfo->src_img_name,"r");
    if(decInfo->fptr_src_image==NULL)
    {
        perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_img_name);
        return e_failure;
    }
    fseek(decInfo->fptr_src_image,54,SEEK_SET);
    return e_success;
}

Status decode_data_from_image(char *data, int size, FILE *fptr_src_image)
{   
    
    
    char ch=0x00;
    char buffer[8];
    for(int i=0;i<size;i++)
    {
        fread(buffer,1,8,fptr_src_image);
        ch=ch&0;
        if(decode_byte_from_lsb(&ch,buffer)==e_success)
        {   
            //printf("hai\n");
            data[i]=ch;
        }
    }
    
    data[size]='\0';
    //printf("\nMS is %s",data);
    return e_success;
}

Status decode_byte_from_lsb(char *ch, char *image_buffer)
{   
    unsigned int mask;
    //char ch1=0;
    for(int i=0;i<8;i++)
    {   
        mask=image_buffer[i]&1;
        *ch= (*ch | (mask<<(7-i)));
    }
    //printf("%c",ch1);
    //char *ptr=&ch1;
    //*ch=ch1;
    //printf("%c",*ch);
    return e_success;
}
Status decode_magic_string(int size,DecodeInfo *decInfo)
{
   char magic_str[size];
   if(decode_data_from_image(magic_str,size,decInfo->fptr_src_image)==e_success)
   {
     printf("INFO: Magic string is %s\n",magic_str);
     if(strcmp(magic_str,MAGIC_STRING)==0)
     {
        decInfo->magic_str=magic_str;
        //printf("%s",decInfo->magic_str);
        return e_success;
     }
     
   }
   return e_failure;
}

Status decode_size_from_lsb(int *size, char *buffer)
{
    uint mask;
    int num=0;
    for(int i=0;i<32;i++)
    {
        mask=buffer[i] & 1;
        num=num | (mask<<(31-i));
    }
    *size=num;
    return e_success;
}

Status decode_extn_size(FILE *fptr_src_image, DecodeInfo *decInfo)
{
    char buffer[32];
    int extn_size;
    fread(buffer,1,32,fptr_src_image);
    if(decode_size_from_lsb(&extn_size,buffer)==e_success)
    {
        decInfo->extn_size=extn_size;
        return e_success;
    }
}

Status decode_extn(int size, DecodeInfo *decInfo)
{
    char extn[size];
    if(decode_data_from_image(extn,size,decInfo->fptr_src_image)==e_success)
    {
        strcpy(decInfo->extn_name,extn);
        return e_success;
    }
}

Status create_decode_dest_file(char *file_name,char *extn_name,DecodeInfo *decInfo)
{   
    
    //int size=strlen(file_name)+strlen(extn_name);
    //printf("Extension is %s\n",extn_name);
    char ptr[50];
    strcpy(ptr,file_name);
    
    strcat(ptr,extn_name);
    //printf("Destination file is %s\n",ptr);
    decInfo->dest_file_name=ptr;
    decInfo->dest_file=fopen(decInfo->dest_file_name,"w");
    if(decInfo->dest_file==NULL)
    {
        return e_failure;
    }
    printf("INFO: Destination file is %s\n",decInfo->dest_file_name);
    return e_success;
}

Status decode_file_size(FILE *fptr_src_image, DecodeInfo *decInfo)
{
    char buffer[32];
    int file_size;
    fread(buffer,1,32,fptr_src_image);
    if(decode_size_from_lsb(&file_size,buffer)==e_success)
    {
        decInfo->file_size=file_size;
        return e_success;
    }
}

Status decode_file_data(int size, DecodeInfo *decInfo)
{
    char data1[size];
    if(decode_data_from_image(data1,size,decInfo->fptr_src_image)==e_success)
    {
        printf("INFO: The data is %s\n",data1);
        fwrite(data1,1,size,decInfo->dest_file);
        return e_success;
    }
    return e_failure;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO: Decoding...\n");
    if(open_files_decoding(decInfo)==e_success)
    {
       printf("INFO: Opening files\n");
       if(decode_magic_string(MAGIC_STRING_SIZE,decInfo)==e_success)
       {
        printf("INFO: Magic String Decoded\n");
       }
       else{
        printf("ERROR: Magic String is not decoded\n");
       }
       if(decode_extn_size(decInfo->fptr_src_image,decInfo)==e_success)
       {
        printf("INFO: Extension size is %d\n",decInfo->extn_size);
        if(decode_extn(decInfo->extn_size,decInfo)==e_success)
        {
            printf("INFO: Extension is %s\n",decInfo->extn_name);
            if(create_decode_dest_file(decInfo->dest_file_name,decInfo->extn_name,decInfo)==e_success)
            {
                printf("INFO: New file created\n");
                if(decode_file_size(decInfo->fptr_src_image,decInfo)==e_success)
                {
                    printf("INFO: File size is %d\n",decInfo->file_size);
                    if(decode_file_data(decInfo->file_size,decInfo)==e_success)
                    {
                        printf("INFO: Data is fetched\n");
                    }
                    else{
                        printf("ERROR: File Data is not decoded\n");
                    }
                }
                else{
                    printf("ERROR: File size is not decoded\n");
                }
            }
            else
            {
                printf("ERROR: Output file is not created\n");
            }
        }
        else{
            printf("ERROR: Extension size is not decoded!\n");
        }
       }
       else{
        printf("ERROR: Extension size not decoded\n");
       }
    }
    else{
        printf("ERROR: Opening files is not successful!\n");
    }
}
