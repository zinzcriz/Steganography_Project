#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include <stdlib.h>
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    uint img_size;

    if(argc<3)
    {
        printf("ERROR: Argument count should be atleast 3\n");
        exit(0);
    }
    if(e_encode == check_operation_type(argv))
    {
        if (argc<4)
        {
            printf("ERROR: For encoding Argument count should be atleast 4\n");
            exit(0);
        }
        
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {   
            printf("INFO: You opted for encoding\n");
            printf("INFO: Argument validation for encoding Successfull\n");
            do_encoding(&encInfo);
        }
    }
    else if(e_decode == check_operation_type(argv))
    {
        
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            // do_encoding(&encInfo);
            printf("INFO: You opted for decoding\n");
            printf("INFO: Argument validation for decoding Successfull\n");
            do_decoding(&decInfo);
        }
    }
    // else if()

    return 0;
}
OperationType check_operation_type(char *argv[])
{
    /*Check the argv[1] is '-e' or not*/
    if(!strcmp("-e", argv[1]))
    {
        return e_encode;
    }

    /*Check the argv[1] is '-d' or not*/
    else if(!strcmp("-d", argv[1]))
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
