#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include<string.h>

#include "types.h" 

#define MAGIC_STRING_SIZE 2

typedef struct _DecodeInfo
{
    char *src_img_name;
    FILE *fptr_src_image;

    char *dest_file_name;
    FILE *dest_file;
    int file_size;
    char extn_name[4];
    int extn_size;

    char *magic_str;

    char *temp;
    
}DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status open_files_decoding(DecodeInfo *decInfo);

Status decode_magic_string(int size,DecodeInfo *decInfo);

Status decode_data_from_image(char *data, int size, FILE *fptr_src_image);

Status decode_byte_from_lsb(char *ch, char *image_buffer);

Status decode_size_from_lsb(int *size, char *buffer);

Status decode_extn_size(FILE *fptr_src_image, DecodeInfo *decInfo);

Status decode_extn(int size, DecodeInfo *decInfo);

Status create_decode_dest_file(char *file_name,char *extn_name,DecodeInfo *decInfo);

Status decode_file_size(FILE *fptr_src_image, DecodeInfo *decInfo);

Status decode_file_data(int size, DecodeInfo *decInfo);
//Status create_decode_dest_file(DecodeInfo *decInfo);

#endif