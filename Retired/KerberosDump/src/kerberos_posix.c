#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#if defined(__linux__)
#include <krb5/krb5.h>
#else
/* Mac OSX framework headers */
#include <Kerberos/Kerberos.h>
#endif

#include "debug.h"
#include "b64.h"
#include "ccache.h"

#if defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

void printFlags(int flag){
    printf("Flags: ");
    /* Find a better way of doing this.... Cause this is dumb... */
    if (flag & TKT_FLG_FORWARDABLE){
        printf("FORWARDABLE | ");
    }
    if (flag & TKT_FLG_FORWARDED){
        printf("FORWARDED | ");
    }
    if (flag & TKT_FLG_PROXIABLE){
        printf("PROXIABLE | ");
    }
    if (flag & TKT_FLG_PROXY){
        printf("PROXY | ");
    }
    if (flag & TKT_FLG_MAY_POSTDATE){
        printf("MAY_POSTDATE | ");
    }
    if (flag & TKT_FLG_INVALID){
        printf("INVALID | ");
    }
    if (flag & TKT_FLG_RENEWABLE){
        printf("RENEWABLE | ");
    }
    if (flag & TKT_FLG_INITIAL){
        printf("INITIAL | ");
    }
    if (flag & TKT_FLG_PRE_AUTH){
        printf("PRE_AUTH | ");
    }
    if (flag & TKT_FLG_HW_AUTH){
        printf("HW_AUTH | ");
    }
    if (flag & TKT_FLG_TRANSIT_POLICY_CHECKED){
        printf("TRANSIT_POLICY_CHECKED | ");
    }
    if (flag & TKT_FLG_OK_AS_DELEGATE){
        printf("OK_AS_DELEGATE | ");
    }
    if (flag & TKT_FLG_ANONYMOUS){
        printf("ANONYMOUS | ");
    }
    printf("\n");
}

int removePrincipal(unsigned char* cacheName, unsigned char* principalName){
    krb5_error_code ret = 0;
    krb5_error_code ret2 = 0;
    krb5_context context;
    krb5_ccache cache;
    krb5_cc_cursor cursor;
    krb5_creds creds;
    char* principalName2 = NULL;
    if (cacheName == NULL || principalName == NULL){
        return 1;
    }
    ret = krb5_init_context(&context);
    if (ret == 0){
        ret = krb5_cc_resolve(context, (char*)cacheName, &cache);
        if (ret == 0){
            ret = krb5_cc_start_seq_get(context, cache, &cursor);
            if (ret == 0){
                while ((ret = krb5_cc_next_cred(context, cache, &cursor, &creds)) == 0){
                    krb5_unparse_name(context, creds.server, &principalName2);
                    if (strcmp((char*)principalName, principalName2) == 0){
                        printf("Removing the cred\n");
                        ret2 = krb5_cc_remove_cred(context, cache, 8, &creds);
                        if (ret2 != 0){
                            printf("ERROR: %d Failed to remove cred.\n", ret2);
                        }
                    }
                    krb5_free_unparsed_name(context, principalName2);
                    krb5_free_cred_contents(context, &creds);
                }
                krb5_cc_end_seq_get(context, cache, &cursor);
            }
            krb5_cc_close(context, cache);
        }
        krb5_free_context(context);
    }
    return 0;
}

int dumpTickets(void){
    krb5_error_code ret = 0;
    krb5_context context;
    krb5_cc_cursor cursor;
    krb5_ccache id;
    krb5_creds creds;
    int counter = 0;
    char* client = NULL;
    char* principalName = NULL;
    char timevalue[255] = {0};
    time_t temptime = 0;
    struct tm *tempTM = 0;
    char* encodeddata = NULL;
    unsigned char* ccacheOutput = NULL;
    int ccacheOutputLen = 0;
    ret = krb5_init_context(&context);
    if (ret == 0){
        printf("Success\n");
        ret = krb5_cc_default(context, &id);
        //ret = krb5_cc_resolve(context, ccache/*name here*/, &id);
        if (ret == 0){
            ret = krb5_cc_start_seq_get(context, id, &cursor);
            if (ret == 0){
                while ((ret = krb5_cc_next_cred(context, id, &cursor, &creds)) == 0){
                    krb5_unparse_name(context, creds.client, &client);
                    printf("\nClient: %s\n", client);
                    krb5_free_unparsed_name(context, client);
                    krb5_unparse_name(context, creds.server, &principalName);
                    printf("Principal: %s\n", principalName);
                    if (creds.keyblock.enctype == ENCTYPE_AES128_CTS_HMAC_SHA1_96){
                        printf("Key enctype: aes128\n");
                    }
                    else if (creds.keyblock.enctype == ENCTYPE_DES3_CBC_SHA1){
                        printf("Key enctype: des3\n");
                    }
                    else if (creds.keyblock.enctype == ENCTYPE_AES256_CTS_HMAC_SHA1_96){
                        printf("Key enctype: aes256\n");
                    }
                    else if (creds.keyblock.enctype == ENCTYPE_ARCFOUR_HMAC){
                        printf("Key enctype: rc4\n");
                    }
                    else{
                        printf("Key enctype: %d\n", creds.keyblock.enctype);
                    }
                    for (counter = 0; counter < creds.keyblock.length; counter++){
                        printf("%02X", creds.keyblock.contents[counter]);
                    }
                    printf("\n");
                    /* Convert creds.times.starttime, creds.times.renew_till, and creds.times.endtime to actual time formats */
                    temptime = creds.times.starttime;
                    tempTM = gmtime(&temptime);
                    if (strftime(timevalue, sizeof(timevalue), "%Y%m%d%H%M%S %z", tempTM) == 0){
                        printf("failed\n");
                    }
                    printf("Start Time: %s\n", timevalue);
                    memset(timevalue, 0, sizeof(timevalue));

                    temptime = creds.times.renew_till;
                    tempTM = gmtime(&temptime);
                    if (strftime(timevalue, sizeof(timevalue), "%Y%m%d%H%M%S %z", tempTM) == 0){
                        printf("failed\n");
                    }
                    printf("Renew Till: %s\n", timevalue);
                    memset(timevalue, 0, sizeof(timevalue));
                    
                    temptime = creds.times.endtime;
                    tempTM = gmtime(&temptime);
                    if (strftime(timevalue, sizeof(timevalue), "%Y%m%d%H%M%S %z", tempTM) == 0){
                        printf("failed\n");
                    }
                    printf("End Time: %s\n", timevalue);
                    memset(timevalue, 0, sizeof(timevalue));
                    /* creds.ticket_flags, print here*/
                    printFlags(creds.ticket_flags);
                    /* Also iterate over authdata list and print out "contents" for "length" of each.*/
                    /* then print out creds.ticket.data with creds.ticket.length (if "X-CACHECONF" in string)
                     * otherwise create a kirbi, and print that out (base64 encoded, which means need b64 encoder func) */
                    if (strstr(principalName, "X-CACHECONF") != NULL){
                        printf("Principal type: %s\n", (creds.server->data)[1].data);
                        encodeddata = b64_encode((unsigned char*)creds.ticket.data, creds.ticket.length);
                        if (encodeddata != NULL){
                            printf("Ticket Data: %s\n", encodeddata);
                            free(encodeddata);
                            encodeddata = NULL;
                        }
                    }
                    else {
                        printf("Kirbi (Supposed to be, but dumping ccache for now)\n");
                        ccacheOutput = generate_ccache_binary(&(creds), &ccacheOutputLen);
                        if (ccacheOutput != NULL){
                            encodeddata = b64_encode((unsigned char*)ccacheOutput, ccacheOutputLen);
                            if (encodeddata != NULL){
                                printf("Ticket Data: %s\n", encodeddata);
                                free(encodeddata);
                                encodeddata = NULL;
                            }
                            free(ccacheOutput);
                            ccacheOutput = NULL;
                        }
                    }
                    krb5_free_unparsed_name(context, principalName);
                    krb5_free_cred_contents(context, &creds);
                }
                ret = krb5_cc_end_seq_get(context, id, &cursor);
                if (ret){
                    printf("Error ending sequence\n");
                }
            }
            /* If adding key from creds, then you need to do this
             * ret = krb5_cc_destroy(context, id);
             * if (ret){printf("Error\n");}
             * else{printf("Successfully removed CCache entry\n");}
             */

            krb5_cc_close(context, id);
        }
        krb5_free_context(context);
    }

    return 0;
}

int listTickets(void){
    krb5_error_code ret = 0;
    krb5_context context;
    krb5_cccol_cursor cursor;
    krb5_cc_cursor cc_cursor;
    krb5_ccache entry;
    krb5_principal principal;
    krb5_creds creds;
    const char* newName = NULL;
    char* principalString = NULL;
    char* servername = NULL;
    char* clientname = NULL;
    
    ret = krb5_init_context(&context);
    if (ret == 0){
        printf("Success\n");
        krb5_cccol_cursor_new(context, &cursor);
        while ((ret=krb5_cccol_cursor_next(context, cursor, &entry)) == 0){
            if (entry == NULL){
                printf("EntryPointer is NULL, breaking out.\n");
                break;
            }
            newName = krb5_cc_get_type(context, entry);
            printf("Name: %s\n", newName);
            if (strcmp(newName, "FILE") == 0){
                printf("KRB5CCNAME: %s\n", getenv("KRB5CCNAME"));
            }
        
 
            ret = krb5_cc_get_principal(context, entry, &principal);
            if (ret){
                printf("ERROR: continuing\n");
                continue;
            }
            principalString = NULL;
            krb5_unparse_name(context, principal, &principalString);
            printf("[*] Principal: %s\n\tName: %s\n", principalString, newName);
            krb5_free_unparsed_name(context, principalString);
            
            ret = krb5_cc_start_seq_get(context, entry, &cc_cursor);
            if (ret){
                printf("ERROR: continuing\n");
                break;
            }
            while((ret = krb5_cc_next_cred(context, entry, &cc_cursor, &creds)) == 0){
                krb5_unparse_name(context, creds.server, &servername);
                krb5_unparse_name(context, creds.client, &clientname);
                printf("\tServer: %s\n", servername);
                printf("\tClient: %s\n", clientname);
                krb5_free_unparsed_name(context, servername);
                krb5_free_unparsed_name(context, clientname);
                /* Print startTime and endTime, and free the creds here*/
                krb5_free_cred_contents(context, &creds);
            }
            krb5_free_principal(context, principal);
            krb5_cc_end_seq_get(context, entry, &cc_cursor);
            krb5_cc_close(context, entry);
        }
        krb5_cccol_cursor_free(context, &cursor);
        krb5_free_context(context);
    }
    printf("Returning\n");
    return 0;
}

void importTicket(unsigned char* indata, char* cachename){
    unsigned char* decodedTicket = NULL;
    krb5_creds* outccache = NULL;
    krb5_context context;
    krb5_ccache cache;
    krb5_error_code ret = 0;
#if defined(__APPLE__)
    krb5_principal test = NULL;
    krb5_principal test2 = NULL;
    unsigned char* origclient = NULL;
    unsigned char* origserver = NULL;
#endif
    
    decodedTicket = b64_decode((char*)indata, strlen((char*)indata));
    outccache = parse_ccache(decodedTicket, strlen((char*)indata));
    if (outccache != NULL){
        DEBUG_PRINT("It parsed!!\n");
        ret = krb5_init_context(&context);
        if (ret == 0){
            DEBUG_PRINT("Initialized\n");
#if defined(__APPLE__)
            if (outccache->client->length == 1){
                ret = krb5_build_principal(context, &test, strlen(outccache->client->realm.data), outccache->client->realm.data, outccache->client->data[0].data, NULL);
                DEBUG_PRINT("Replacing client\n");
            }
            if (outccache->server->length == 1){
                ret = krb5_build_principal(context, &test2, strlen(outccache->server->realm.data), outccache->server->realm.data, outccache->server->data[0].data, NULL);
                DEBUG_PRINT("replacing server with 1\n");
            }
            else if (outccache->server->length == 2){
                ret = krb5_build_principal(context, &test2, strlen(outccache->server->realm.data), outccache->server->realm.data, outccache->server->data[0].data, outccache->server->data[1].data, NULL);
                DEBUG_PRINT("replacing server with 2\n");
            }
            if (test == NULL || test2 == NULL){
                printf("Failed to initialize the client and server principal names\n");
                goto cleanup;
            }
            origclient = (unsigned char*)outccache->client;
            origserver = (unsigned char*)outccache->server;
            outccache->client = test;
            outccache->server = test2;
#endif

            if (cachename != NULL && (strcmp(cachename, "new") == 0)){
                DEBUG_PRINT("Using New\n");
#if defined(__APPLE__)
                ret = krb5_cc_new_unique(context, "API", "test", &cache);
#else
                ret = krb5_cc_new_unique(context, "FILE", cachename, &cache);
#endif
                DEBUG_PRINT("after new\n");
                if (ret != 0){
                    printf("failed to create new cache with error %d, exiting.", ret);
                    krb5_free_context(context);
                    goto cleanup;
                }
                DEBUG_PRINT("Trying to initialize\n");

                ret = krb5_cc_initialize(context, cache, outccache->client);
                
                if (ret != 0){
                    printf("Failed to initialize: %d\n", ret);
                }
                printf("Initialized\n");
            }
            else{
                printf("Using the resolved name:%s\n", cachename);
                ret = krb5_cc_resolve(context, cachename, &cache);
            }
            if (ret == 0){
                printf("Trying to store cred\n");
                ret = krb5_cc_store_cred(context, cache, outccache);
                if (ret != 0){
                    printf("Error storing cred: %d rerun with \"new\" as the cachename with new again.\n", ret);
                }
                printf("Stored correctly, closing\n");
                krb5_cc_close(context, cache);
            }
            krb5_free_context(context);
        }
        /* Do freeing of krb5_cred here (my internal function, since I don't know how krb5's free works. */
    }

cleanup:
    if (outccache != NULL){
#if defined(__APPLE__)
        outccache->client = (krb5_principal)origclient;
        outccache->server = (krb5_principal)origserver;
        krb5_free_principal(context, test);
        krb5_free_principal(context, test2);
#endif
        if (freeStructure(outccache) == 0){
            DEBUG_PRINT("Freed the credstruct\n");
        }
    }
    if (decodedTicket){
        free(decodedTicket);
    }
    DEBUG_PRINT("Done\n");
}

int main(int argc, char* argv[]){
    int option;
    char* name = NULL;
    char* cache = NULL;
    char* ticket = NULL;
    // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
    while((option = getopt(argc, argv, ":in:c:t:rldh")) != -1){ //get option from the getopt() method
        switch(option){
            //For option i, r, l, print that these are options
            case 'i':
                printf("Doing import\n");
                importTicket((unsigned char*)ticket, cache);
                break;
            case 'l':
                printf("Listing Tickets\n");
                (void)listTickets();
                break;
            case 'd':
                printf("Dumping Tickets\n");
                (void)dumpTickets();
                break;
            case 'r':
                if (name != NULL && cache != NULL){
                    printf("Removing %s from %s\n", name, cache);
                    removePrincipal((unsigned char*)cache, (unsigned char*)name);
                }
                break;
            case 'n': //here f is used for some file name
                printf("Given name argument: %s\n", optarg);
                name = optarg;
                break;
            case 'c':
                printf("Using cache: %s\n", optarg);
                cache = optarg;
                break;
            case 't':
                printf("Ticket data: %s\n", optarg);
                ticket = optarg;
                break;
           case ':':
                printf("option needs a value\n");
                break;
            case '?': //used for some unknown options
                printf("unknown option: %c\n", optopt);
            case 'h':
                printf("Options : \n\t-i - import ticket (name, cache, and ticket needed)\n\t-l - list tokens\n\t-d - dump tokens\n\t-r - remove principal from cache\n\t-h - (this)\n\t-n - name option, used if removing a name\n\t-c - cache option, used for removing and importing\n\t-t - b64 ticket data\n");
                break;
        }
    }
    for(; optind < argc; optind++){ //when some extra arguments are passed
        printf("Given extra arguments: %s\n", argv[optind]);
   }

    return 0;
}
#if defined(__APPLE__)
#pragma clang diagnostic pop
#endif
