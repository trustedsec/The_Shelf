#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <GSS/GSS.h>

#include "b64.h"

typedef struct _mykrb5_data {
    uint32_t magic;
    unsigned int length;
    char *data;
} mykrb5_data;

typedef struct my_krb5_ccache_data {
    const struct krb5_cc_ops *ops;
    mykrb5_data data;
    unsigned int cc_initialized:1;      /* if 1: krb5_cc_initialize() called */
    unsigned int cc_need_start_realm:1;
    unsigned int cc_start_tgt_stored:1;
    unsigned int cc_kx509_done:1;
}my_krb5_ccache_data;

void iterfunc(void* ctx, gss_OID in_oid, gss_cred_id_t credidHere){
    uint32_t lifetime = 0;
    uint32_t retcode = 0;
    uint32_t minor_status = 0;
    char* encodeddata = NULL;
    gss_buffer_t exportedbuffer;
    my_krb5_ccache_data *ccachedata;
    ccachedata = calloc(sizeof(my_krb5_ccache_data)+1000, 1);
    memset(ccachedata, 0, sizeof(my_krb5_ccache_data));
    exportedbuffer = calloc(sizeof(gss_buffer_desc), 1);
    exportedbuffer->length = 0;
    exportedbuffer->value = NULL;
    printf("Inside iterfunc\n");
    if (credidHere){
        lifetime = GSSCredentialGetLifetime(credidHere);
        if (retcode == GSS_S_COMPLETE){
            printf("Got data\n");
            retcode = gss_export_cred(&minor_status, credidHere, exportedbuffer);
            if (retcode == GSS_S_COMPLETE){
                printf("Successfully exported cred\n");
                encodeddata = b64_encode((unsigned char*)exportedbuffer->value, exportedbuffer->length);
                printf("Exported ticket:\n%s\n", encodeddata);
            }
            #if 0
            printf("Trying to copy ccache\n");
            retcode = gss_krb5_copy_ccache(&minor_status, credidHere, ccachedata);
            if (retcode == GSS_S_COMPLETE){
                printf("Success\n");
                encodeddata = b64_encode((unsigned char*)ccachedata, sizeof(my_krb5_ccache_data));
                printf("CCACHE:\n%s\n", encodeddata);
                printf("Magic:\t%d\n", ccachedata->data.magic);
                encodeddata = b64_encode((unsigned char*) ccachedata->data.data, ccachedata->data.length);
                printf("krb5_data:\n%s\n", encodeddata);
            }
            #endif
            printf("Lifetime: %d\n", lifetime);
        }
    }

}

int main(void){
    uint32_t retcode = 0;
    uint32_t minstatus = 0;
    
    retcode = gss_iter_creds_f(&minstatus, 0, NULL, NULL, iterfunc);
    printf("Back\n");
    return 0;
}
