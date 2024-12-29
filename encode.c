#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(strstr(argv[2],".bmp")!=NULL)
    {
        encInfo->src_image_fname=argv[2];
    }
    else
    {   
        printf("ERROR: The third argument should contain a .bmp file\n");
        return e_failure;
    }
    if(strstr(argv[3],".txt")!=NULL)
    {
        encInfo->secret_fname=argv[3];
        strcpy(encInfo->extn_secret_file,".txt");
    }
    else if(strstr(argv[3],".sh")!=NULL)
    {
        encInfo->secret_fname=argv[3];
        strcpy(encInfo->extn_secret_file,".sh");
    }
    else if(strstr(argv[3],".c")!=NULL)
    {
        encInfo->secret_fname=argv[3];
        strcpy(encInfo->extn_secret_file,".c");
    }
    else
    {  
        printf("ERROR: Argument 4 should contain a .txt/.sh/.c file\n");
        return e_failure;
    }
    if(argv[4]==NULL)
    {
        encInfo->stego_image_fname="stego_img.bmp";
    }
    else{
        encInfo->stego_image_fname=argv[4];
    }
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char headerfile[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(headerfile,1,54,fptr_src_image);
    fwrite(headerfile,1,54,fptr_dest_image);
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{   
    char buffer[8];
    for(int i=0;i<size;i++)
    {   
        fread(buffer,1,8,fptr_src_image);
        if(encode_byte_to_lsb(data[i],buffer)==e_success)
        {
            fwrite(buffer,1,8,fptr_stego_image);
        }
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{   
    uint mask;
    for(int i=0;i<8;i++)
    {
        mask=(data&(1<<(7-i)));
        mask=mask>>(7-i);
        image_buffer[i]=(image_buffer[i] & ~(1)) | mask;
    }
    return e_success;
}
Status encode_size_to_lsb(int size, char *buffer)
{
    uint mask;
    for(int i=0;i<32;i++)
    {
       mask=(size&(1<<(31-i)));
       mask=mask>>(31-i);
       buffer[i]=(buffer[i] & ~(1)) | mask;
    }
    return e_success;
}

Status encode_size_secret_file_extn(int size, FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char image_buffer[32];
    fread(image_buffer,1,32,fptr_src_image);
    if(encode_size_to_lsb(size,image_buffer)==e_success)
    {
        fwrite(image_buffer,1,32,fptr_dest_image);
        return e_success;
    }
    else{
        return e_failure;
    }
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if(encode_data_to_image(file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
    {
        return e_success;
    }
    return e_failure;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{   
    //int size=get_file_size(encInfo->fptr_secret);
    char img_buffer[32];
    fread(img_buffer,1,32,encInfo->fptr_src_image);
    if(encode_size_to_lsb(file_size,img_buffer)==e_success)
    {
        fwrite(img_buffer,1,32,encInfo->fptr_stego_image);
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{   
    int size=get_file_size(encInfo->fptr_secret);
    fseek(encInfo->fptr_secret,0,SEEK_SET);
    //char *buffer=malloc(size);
    char buffer[size];
    fread(buffer,1,size,encInfo->fptr_secret);
    if(encode_data_to_image(buffer,size,encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
    {   
        //free(buffer);
        return e_success;
    }
    return e_failure;
}


Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer;
    size_t bytes_read;

    //fseek(fptr_src, ftell(fptr_src), SEEK_SET);

    while ((bytes_read = fread(&buffer, 1, 1, fptr_src)) > 0)
    {
        fwrite(&buffer, 1, 1, fptr_dest);
    }

    return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{   
    printf("INFO: Opening required files...\n");
    if(open_files(encInfo)==e_success)
    {
        if(check_capacity(encInfo)==e_success)
        {
            printf("INFO: Capacity check successful\n");
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
            {
                printf("INFO: Header file copied successfully\n");
                if(encode_magic_string(MAGIC_STRING,encInfo)==e_success)
                {
                    printf("INFO: Magic String is encoded\n");
                    if(encode_size_secret_file_extn(strlen(encInfo->extn_secret_file),encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
                    {
                        printf("INFO: Size of secret file extension is encoded\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_success)
                        {
                            printf("INFO: Extension file encoded\n");
                        }
                        else
                        {
                            printf("ERROR: Extension file is not encoded\n");
                        }
                        if(encode_secret_file_size(get_file_size(encInfo->fptr_secret),encInfo)==e_success)
                        {
                            printf("INFO: Secret file size is encoded\n");
                        }
                        else{
                            printf("ERROR: Secret file size is not encoded\n");
                        }
                        if(encode_secret_file_data(encInfo)==e_success)
                        {
                            printf("INFO: Secret file is encoded\n");
                        }
                        else
                        {
                            printf("ERROR: SEcret file is not encoded\n");
                        }
                        if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
                        {
                            printf("INFO: Copied remaining image\n");
                        }

                    }
                    else
                    {
                        printf("ERROR: Failed to encode extension size\n");
                    }
                }
                else
                {
                    printf("ERROR: Header File not copied!\n");
                }
            }
            else
            {
                printf("ERROR: Header files not copied successfully!\n");
            }
        }
        else{
            printf("ERROR: capacity is not sufficient\n");
        }
    }
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
   if(encode_data_to_image(magic_string,sizeof(MAGIC_STRING)-1,encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
   {
    return e_success;
   }
   return e_failure;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}


uint get_file_size(FILE *fptr_secret_file){
    //Seek to end of file
    fseek(fptr_secret_file,0L,SEEK_END);

    // Return the file size 
    return ftell(fptr_secret_file);
}

Status check_capacity(EncodeInfo *encInfo)
{
    printf("INFO: Checking capacity of %s for storing %s",encInfo->src_image_fname,encInfo->secret_fname);
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    uint bmp_header_size=54;
    uint magic_string_size=2*8;

    uint secret_file_extn_size=strlen(encInfo->extn_secret_file)*8;
    uint secret_file_extn=strlen(encInfo->extn_secret_file)*8;

    uint secret_file_data_size=sizeof(encInfo->size_secret_file)*8;
    uint secret_file_data=(encInfo->size_secret_file)*8;

    uint expected_capacity=bmp_header_size+magic_string_size+secret_file_extn_size+secret_file_extn+secret_file_data_size+secret_file_data;
    if(encInfo->image_capacity>expected_capacity)
    {  
        printf("INFO: Enough Capacity is there for storing\n");
        return e_success;
    }
    else
    {  
        return e_failure;
    }
}
/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
