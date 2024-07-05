#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(__linux__)
#include <arpa/inet.h>
#include <krb5/krb5.h>
#else
/* Mac OSX framework headers */
#include <Kerberos/Kerberos.h>
#endif

#include "debug.h"
#include "ccache.h"

/* Generation Code Starts Here */
unsigned char* get_principal_struct(unsigned char* data, int *outstructlen){
    principal *principalOffsets = NULL;
    counted_octet_string *cosPTR = NULL;
    unsigned char* offsetLocation = NULL;
    int counter = 0;
    //krb5_creds* creds = NULL;
    /*Typedef for krb5_principal is a pointer*/
    krb5_principal principalstruct = NULL;
    
    int allocatedLen = 0;
    principalstruct = (krb5_principal)data;
    /* Adding 255 to this incase I screw up something horribly, will remove once I know its accurate without it....*/
    allocatedLen = sizeof(principal) + principalstruct->realm.length + principalstruct->data[0].length + 1024;
    /* Set the principal length, and fill out the struct here */
    principalOffsets = calloc(allocatedLen, 1);
    if (principalOffsets == NULL){
        DEBUG_PRINT("Failed to allocate memory\n");
        return NULL;
    }
    
    
    principalOffsets->name_type = principalstruct->type;
    principalOffsets->name_type = htonl(principalOffsets->name_type);
    //DEBUG_PRINT("Length of principalstruct Hoping 0 or 1...: %d\n", (int)(principalstruct->length));
    principalOffsets->num_components = principalstruct->length;
    principalOffsets->num_components = htonl(principalOffsets->num_components);
    principalOffsets->realm.length = principalstruct->realm.length;
    principalOffsets->realm.length = htonl(principalOffsets->realm.length);
    memcpy(principalOffsets->realm.data, principalstruct->realm.data, principalstruct->realm.length);
    offsetLocation = (unsigned char*)principalOffsets->realm.data;
    offsetLocation = offsetLocation+principalstruct->realm.length;
    for (counter = 0; counter < principalstruct->length; counter++){
        cosPTR = (counted_octet_string*)offsetLocation;
        cosPTR->length = principalstruct->data[counter].length;
        memcpy(cosPTR->data, principalstruct->data[counter].data, principalstruct->data[counter].length);
        offsetLocation = cosPTR->data;
        offsetLocation = offsetLocation + cosPTR->length;
        cosPTR->length = htonl(cosPTR->length);
    }
    *outstructlen = (offsetLocation - (unsigned char*)principalOffsets);
    return (unsigned char*)principalOffsets;
}


unsigned char* get_counted_octet_string(unsigned char* data, int *outstructlen){
    krb5_data *indata = NULL;
    counted_octet_string* outdata = NULL;
    int outdataLen = 0;
    indata = (krb5_data*)data;
    outdataLen = sizeof(counted_octet_string) + indata->length;
    outdata = calloc(outdataLen, 1);
    outdata->length = indata->length;
    outdata->length = htonl(outdata->length);
    memcpy(outdata->data, indata->data, indata->length);
    /* Think I may need to subtract 1 from these since I have the arrays as 
     * arrays of len 1 everywhere.*/
    *outstructlen = outdataLen;
    return (unsigned char*)outdata;
}

unsigned char* get_times_struct(unsigned char* data, int *outstructlen){
    krb5_ticket_times *indata = NULL;
    times *outdata = NULL;
    int outdataLen = 0;

    indata = (krb5_ticket_times*)data;
    outdataLen = sizeof(times);
    outdata = calloc(outdataLen, 1);
    if (outdata == NULL){
        return NULL;
    }
    outdata->authtime = htonl(indata->authtime);
    outdata->starttime = htonl(indata->starttime);
    outdata->endtime = htonl(indata->endtime);
    outdata->renew_till = htonl(indata->renew_till);
    
    *outstructlen = outdataLen;
    return (unsigned char*)outdata;
}

unsigned char* get_keyblock_struct(unsigned char* data, int *outstructlen){
    krb5_keyblock *indata = NULL;
    keyblock *outdata = NULL;
    int outdataLen = 0;
    indata = (krb5_keyblock *)data;
    outdataLen = sizeof(keyblock) - 1 + indata->length;
    outdata = calloc(outdataLen+1, 1);
    if (outdata == NULL){
        return NULL;
    }
    outdata->keytype = htons(indata->enctype);
    outdata->keylen = htons(indata->length);
    memcpy(outdata->keyvalue, indata->contents, indata->length);
    *outstructlen = outdataLen;
    return (unsigned char*)outdata;
}

unsigned char* get_auth_struct(unsigned char* data, int *outstructlen){
    krb5_authdata *auth = NULL;
    authdata *outdata = NULL;
    int outdataLen = 0;
    
    auth = (krb5_authdata*)data;
    outdataLen = sizeof(authdata)-1 + auth->length;
    outdata = calloc(outdataLen+1, 1);
    if (outdata == NULL){
        return NULL;
    }
    outdata->authtype = htons(auth->ad_type);
    outdata->authdata.length = htons(auth->length);
    memcpy(&(outdata->authdata.data), auth->contents, auth->length);
    *outstructlen = outdataLen;
    return (unsigned char*)outdata;
}


unsigned char* get_addr_struct(unsigned char* data, int *outstructlen){
    krb5_address *addrs = NULL;
    address *outdata = NULL;
    int outdataLen = 0;

    addrs = (krb5_address*)data;
    outdataLen = sizeof(address) - 1 + addrs->length;
    outdata = calloc(outdataLen, 1);
    if (outdata == NULL){
        return NULL;
    }
    outdata->addrtype = htons(addrs->addrtype);
    outdata->addrdata.length = htons(addrs->length);
    memcpy(outdata->addrdata.data, addrs->contents, addrs->length);
    *outstructlen = outdataLen;
    return (unsigned char*)outdata;
}

unsigned char* get_ccache_main(int *outstructlen){
    ccache_head* outdata = NULL;
    int outdataLen = sizeof(ccache_head)-1 + 0x08;
    outdata = calloc(outdataLen, 1);
    if (outdata == NULL){
        return NULL;
    }
    outdata->file_format_version = htons(0x504);
    outdata->headerlen = htons(0xc);
    outdata->headers[0].tag = htons(0x1);
    outdata->headers[0].taglen = htons(0x8);
    memset(&(outdata->headers[0].tagdata), 0xff, 4);
    memset(&(outdata->headers[0].tagdata)+4, 0x00, 4);
    *outstructlen = outdataLen;
    return (unsigned char*)outdata;
}

unsigned char* get_credential_struct(unsigned char* data, int *outstructlen){
    /* calloc base structure, and then realloc after each call
     * get_principal_struct - client
     * get_principal_struct - server
     * get_keyblock_struct - key
     * get_times_struct - time
     * singlebyte (uint8_t) - is_skey
     * uint32_t - tktflags
     * uint32_t - num_address (might need to parse out the ptr 
     *   array krb5_creds->addresses
     *      for address in list call get_auth_struct
     * uint32_t - num_authdata (same as num_address)
     *  array krb5_creds->authdata
     *      for authdata in array.
     * counted_octet_string - ticket
     * counted_octet_string - second_ticket
     * */
    krb5_creds *creds = NULL;
    unsigned char* outbuffer = NULL;
    unsigned char* retval = NULL;
    unsigned char* tempbuf = NULL;
    int structlen = 0;
    int outbufferlen = 0;
    int outbufferoffset = 0;
    uint32_t ticketflags_temp = 0;
    creds = (krb5_creds*)data;
    
    retval = get_principal_struct((unsigned char*)(creds->client), &structlen);
    outbufferlen = outbufferlen+structlen;
    tempbuf = realloc(outbuffer, outbufferlen+1);
    if (tempbuf == NULL){
        goto ret;
    }
    outbuffer = tempbuf;
    memcpy(outbuffer+outbufferoffset, retval, structlen);
    outbufferoffset = outbufferoffset + structlen;
    if (retval){
        free(retval);
        retval = NULL;
    }

    retval = get_principal_struct((unsigned char*)(creds->server), &structlen);
    outbufferlen = outbufferlen+structlen;
    tempbuf = realloc(outbuffer, outbufferlen+1);
    if (tempbuf == NULL){
        goto ret;
    }
    outbuffer = tempbuf;
    memcpy(outbuffer+outbufferoffset, retval, structlen);
    outbufferoffset = outbufferoffset + structlen;
    if (retval){
        free(retval);
        retval = NULL;
    }

    retval = get_keyblock_struct((unsigned char*)&(creds->keyblock), &structlen);
    outbufferlen = outbufferlen+structlen;
    tempbuf = realloc(outbuffer, outbufferlen+1);
    if (tempbuf == NULL){
        goto ret;
    }
    outbuffer = tempbuf;
    memcpy(outbuffer+outbufferoffset, retval, structlen);
    outbufferoffset = outbufferoffset + structlen;
    if (retval){
        free(retval);
        retval = NULL;
    }

    retval = get_times_struct((unsigned char*)&(creds->times), &structlen);
    outbufferlen = outbufferlen+structlen;
    tempbuf = realloc(outbuffer, outbufferlen+13);
    if (tempbuf == NULL){
        goto ret;
    }
    outbuffer = tempbuf;
    memcpy(outbuffer+outbufferoffset, retval, structlen);
    outbufferoffset = outbufferoffset + structlen;
    if (retval){
        free(retval);
        retval = NULL;
    }
    
    memset(outbuffer+outbufferoffset, creds->is_skey, 1);
    outbufferoffset = outbufferoffset+1;
    outbufferlen = outbufferlen+1;
    ticketflags_temp = htonl(creds->ticket_flags);
    memcpy(outbuffer+outbufferoffset, &ticketflags_temp, sizeof(uint32_t));
    outbufferoffset = outbufferoffset + sizeof(uint32_t);
    outbufferlen = outbufferlen+4;
    /* Setting num_address and num_authdata to 0 for now */
    memset(outbuffer+outbufferoffset, 0, 4);
    outbufferoffset = outbufferoffset + sizeof(uint32_t);
    outbufferlen = outbufferlen+4;
    memset(outbuffer+outbufferoffset, 0, 4);
    outbufferoffset = outbufferoffset + sizeof(uint32_t);
    outbufferlen = outbufferlen+4;
    /* Then write size and ticket, and then zeros for second ticket len */
    retval = get_counted_octet_string((unsigned char*)&(creds->ticket), &structlen);
    outbufferlen = outbufferlen+structlen;
    tempbuf = realloc(outbuffer, outbufferlen+1+9);
    if (tempbuf == NULL){
        goto ret;
    }
    outbuffer = tempbuf;
    memcpy(outbuffer+outbufferoffset, retval, structlen);
    outbufferoffset = outbufferoffset + structlen;
    if (retval){
        free(retval);
        retval = NULL;
    }
    
    memset(outbuffer+outbufferoffset, 0, 4);
    outbufferoffset = outbufferoffset + 4;
    /* The file is off by one, and this is the last entry here, so I'm assuming its this one... */
    outbufferlen = outbufferlen +4 -1;
    
    *outstructlen = outbufferlen;
ret:
    return outbuffer;

//fail:
    /* cleanup here */
//    goto ret;
}


unsigned char* generate_ccache_binary(krb5_creds *creds, int *outdataLen){
    unsigned char *outccache = calloc(0x800, 1);
    int structlen = 0;
    int offsetval = 0;
    unsigned char* returnval = NULL;
    if (outccache == NULL){
        DEBUG_PRINT("Failed to allocate\n");
        return NULL;
    }
    /* get_ccache_main - to set the ccache data
     * get_principal_struct - pimary_principal.
     * get_credential_struct - for each cred
     * */
    returnval = get_ccache_main(&structlen);
    memcpy(outccache+offsetval, returnval, structlen);
    offsetval = offsetval + structlen;
    free(returnval);
    //DEBUG_PRINT("Offsetval == %d\n", offsetval);
    
    returnval = get_principal_struct((unsigned char*)(creds->client), &structlen);
    memcpy(outccache+offsetval, returnval, structlen);
    offsetval = offsetval + structlen;
    free(returnval);
    //DEBUG_PRINT("Offsetval == %d\n", offsetval);
    
    returnval = get_credential_struct((unsigned char*)creds, &structlen);
    memcpy(outccache+offsetval, returnval, structlen);
    offsetval = offsetval + structlen;
    free(returnval);
    //DEBUG_PRINT("Offsetval == %d\n", offsetval);
    *outdataLen = offsetval;
    /* Done with primary_principal, now onto credentials structure.*/
    
    
    return (unsigned char*)outccache;
}

/* Generation Code Stops Here */

/* freeStructure function: 
 * Only use this to free custom krb5_creds structures generated from 
 * parse_ccache. This just simply goes through everything and frees them
 * if you modified the structure restore it so that this works, or you'll
 * have to do it yourself. */

int freeStructure(krb5_creds* instruct){
    int tempcounter = 0;
    if (instruct == NULL){
        return 1;
    }
    /* Free all of the allocated client principal data first */
    if (instruct->client->realm.data){
        free(instruct->client->realm.data);
    }
    for (tempcounter = 0; tempcounter < instruct->client->length; tempcounter++){
        if (instruct->client->data[tempcounter].data){
            free(instruct->client->data[tempcounter].data);
        }
    }
    if (instruct->client->data){
        free(instruct->client->data);
    }
    if (instruct->client){
        free(instruct->client);
    }
    /* Then free all of the server principals data */
    if (instruct->server->realm.data){
        free(instruct->server->realm.data);
    }
    for (tempcounter = 0; tempcounter < instruct->server->length; tempcounter++){
        if (instruct->server->data[tempcounter].data){
            free(instruct->server->data[tempcounter].data);
        }
    }
    if (instruct->server->data){
        free(instruct->server->data);
    }
    if (instruct->server){
        free(instruct->server);
    }
    /* Free the keyblock data */
    if (instruct->keyblock.contents){
        free(instruct->keyblock.contents);
    }
    /* The data arrays allocated in these are lost, in testing I rarely seen 
     * any addresses or authdata fields that actually had any data, so 
     * so I'm just going to free it here and hope for the best.
     * NOTE: Honestly can probably just set addresses and authdata to NULL
     * and just avoid that parsing all together.... */
    if (instruct->addresses){
        free(instruct->addresses);
    }
    if (instruct->authdata){
        free(instruct->authdata);
    }
    /* and last free the ticket data. */
    if (instruct->ticket.data){
        free(instruct->ticket.data);
    }
    /* Then free the structure passed in itself. */
    free(instruct);
    return 0;
};

/* Start Parsing Code Here */

/* Simply skips over a single principal, and returns the buffer location of 
 * the next item in the struct. */
unsigned char* skipPrincipal(unsigned char* buffer){
    int bufferoffset = 0;
    principal* tempPrincipal = (principal*)buffer;
    counted_octet_string* tempOctString = NULL;
    int tempcounter = 0;
    DEBUG_PRINT("Start offset = %p\n", buffer);
    bufferoffset = bufferoffset + sizeof(uint32_t) + sizeof(uint32_t);
    bufferoffset = bufferoffset + sizeof(uint32_t) + ntohl(tempPrincipal->realm.length);
    DEBUG_PRINT("Principal Realm Length: %d\n", ntohl(tempPrincipal->realm.length));
    DEBUG_PRINT("Principal Realm: %s\n", tempPrincipal->realm.data);
    for (tempcounter = 0; tempcounter < ntohl(tempPrincipal->num_components); tempcounter++){
        tempOctString = (counted_octet_string*)(buffer + bufferoffset);
        DEBUG_PRINT("Counter : %d\n", tempcounter);
        DEBUG_PRINT("String: %s\n", tempOctString->data);
        bufferoffset = bufferoffset + sizeof(uint32_t) + ntohl(tempOctString->length);
    }
    DEBUG_PRINT("Returning the new offset\n");
    return buffer + bufferoffset;
}


/* Ok......... FAIR WARNING!!!!!! HERE BE DRAGONS! This code is to parse out
 * a ccache structure, and convert it into a krb5_creds structure. This means
 * the code is huge (hundreds of lines), and it relies on numbers in bigendian
 * format, that gets converted to little endian. It looks bad, and there's no
 * guarantee that it will work with everything, so test with this implementation
 * before using it for real!!. If you want to make it better, I say go for it, 
 * but this code took me a while to build.*/
krb5_creds* parse_ccache(unsigned char* inCcache, int ccacheSize){
    krb5_creds *outcred = NULL;
    unsigned char* offset = NULL;
    ccache* tempccache = NULL;
    principal* tempPrincipal = NULL;
    //credential* tempCred = NULL;
    credentialend* tempCredend = NULL;
    credentialendtwo* tempCredendTwo = NULL;
    keyblock* tempKeyblock = NULL;
    counted_octet_string* tempOctString = NULL;
    int tempcounter = 0;

    outcred = calloc(sizeof(krb5_creds), 1);
    tempccache = (ccache*)inCcache;
    DEBUG_PRINT("Headerlen : %d\n", ntohs(tempccache->headerlen));
    offset = inCcache + sizeof(uint16_t) + sizeof(uint16_t) + ntohs(tempccache->headerlen);
    /* This is the primary principal (or should be) */
    tempPrincipal = (principal*)offset;
    /* Need to skip the primary principal, and parse the creds client principal 
     * NOTE: if you need a primary principal, use the client principal maybe, or else parse it manually.*/
    offset = (unsigned char*)skipPrincipal(offset);
    tempPrincipal = (principal*)offset;
    //tempCred = (credential*) tempPrincipal;
    DEBUG_PRINT("Got new principal %p\n", tempPrincipal);
    outcred->magic = KV5M_CREDS;
    /* Start the client parsing */
    /* set the client type, and realm info. */
    outcred->client = calloc(sizeof(krb5_principal_data), 1);
    DEBUG_PRINT("outcred->client : %p\n", outcred->client);
    outcred->client->magic = KV5M_PRINCIPAL;
    outcred->client->length = ntohl(tempPrincipal->num_components);
    DEBUG_PRINT("Client Length: %d\n", outcred->client->length);
    outcred->client->type = ntohl(tempPrincipal->name_type);
    //outcred->client->type = 0;
    DEBUG_PRINT("Client Type: %d\n", outcred->client->type);
    outcred->client->realm.magic = KV5M_DATA;
    outcred->client->realm.length = ntohl(tempPrincipal->realm.length);
    DEBUG_PRINT("Client realm length: %d\n", outcred->client->realm.length);
    outcred->client->realm.data = calloc(ntohl(tempPrincipal->realm.length)+1, 1);
    if (outcred->client->realm.data == NULL){
        DEBUG_PRINT("failed to allocate memory\n");
        return NULL;
    }
    DEBUG_PRINT("Setting client realm data: %s\n", tempPrincipal->realm.data);
    memcpy(outcred->client->realm.data, tempPrincipal->realm.data, ntohl(tempPrincipal->realm.length));
    DEBUG_PRINT("Set all client data\n");
    /* Set the client components here */
    DEBUG_PRINT("Number of components: %d\n", ntohl(tempPrincipal->num_components));
    outcred->client->data = calloc(sizeof(krb5_data)*(ntohl(tempPrincipal->num_components)+1), 1);
    offset = offset + sizeof(uint32_t) + sizeof(uint32_t);
    /* This is the realm counted_octet_string */
    tempOctString = (counted_octet_string*)offset;
    /* Increase the offset to component location */
    offset = offset + sizeof(uint32_t) + ntohl(tempOctString->length);
    DEBUG_PRINT("Walking over the num_components\n");
    /* set the values here */
    for (tempcounter = 0; tempcounter < ntohl(tempPrincipal->num_components); tempcounter++){
        tempOctString = (counted_octet_string*)offset;
        //DEBUG_PRINT("Base: %p new: %p\n", outcred->client->data, outcred->client->data[tempcounter]);
        //outcred->client->data[tempcounter] = calloc(sizeof(krb5_data), 1);
        DEBUG_PRINT("OctStringLength : %d\n", ntohl(tempOctString->length));
        //outcred->client->data[tempcounter].magic = 1142291616;
        outcred->client->data[tempcounter].magic = KV5M_DATA;
        outcred->client->data[tempcounter].length = ntohl(tempOctString->length);
        DEBUG_PRINT("set the length\n");
        outcred->client->data[tempcounter].data = calloc(ntohl(tempOctString->length) + 1, 1);
        DEBUG_PRINT("Set the data\n");
        if (outcred->client->data[tempcounter].data == NULL){
            DEBUG_PRINT("Failed to allocate memory\n");
            return NULL;
        }
        DEBUG_PRINT("Copying in : %s\n", tempOctString->data);
        memcpy(outcred->client->data[tempcounter].data, tempOctString->data, ntohl(tempOctString->length));
        /* Step over the counted_octet_string */
        offset = offset + sizeof(uint32_t) + ntohl(tempOctString->length);
    }
    
    DEBUG_PRINT("start the server parsing\n");
    /* Start the server parsing */
    tempPrincipal = (principal*)offset;
    outcred->server = calloc(sizeof(krb5_principal_data), 1);
    outcred->server->magic = KV5M_PRINCIPAL;
    outcred->server->length = ntohl(tempPrincipal->num_components);
    DEBUG_PRINT("Server Length: %d\n", outcred->server->length);
    outcred->server->type = ntohl(tempPrincipal->name_type);
    //outcred->server->type = 0;
    DEBUG_PRINT("Server type: %d\n", outcred->server->type);
    outcred->server->realm.magic = KV5M_DATA;
    outcred->server->realm.length = ntohl(tempPrincipal->realm.length);
    DEBUG_PRINT("Server realm Length: %d\n", outcred->server->realm.length);
    outcred->server->realm.data = calloc(ntohl(tempPrincipal->realm.length)+1, 1);
    if (outcred->server->realm.data == NULL){
        DEBUG_PRINT("failed to allocate memory\n");
        return NULL;
    }
    DEBUG_PRINT("realm string: %s\n", tempPrincipal->realm.data);
    memcpy(outcred->server->realm.data, tempPrincipal->realm.data, ntohl(tempPrincipal->realm.length));
    /* Set the client components here */
    outcred->server->data = calloc(sizeof(krb5_data)*(ntohl(tempPrincipal->num_components)+1), 1);
    offset = offset + sizeof(uint32_t) + sizeof(uint32_t);
    /* This is the realm counted_octet_string */
    tempOctString = (counted_octet_string*)offset;
    /* Increase the offset to component location */
    offset = offset + sizeof(uint32_t) + ntohl(tempOctString->length);
    /* set the values here */
    for (tempcounter = 0; tempcounter < ntohl(tempPrincipal->num_components); tempcounter++){
        tempOctString = (counted_octet_string*)offset;
        DEBUG_PRINT("Server Component Len : %d\n", ntohl(tempOctString->length));
        outcred->server->data[tempcounter].magic = KV5M_DATA;
        outcred->server->data[tempcounter].length = ntohl(tempOctString->length);
        outcred->server->data[tempcounter].data = calloc(ntohl(tempOctString->length) + 1, 1);
        if (outcred->server->data[tempcounter].data == NULL){
            DEBUG_PRINT("Failed to allocate memory\n");
            return NULL;
        }
        DEBUG_PRINT("Copying in : %s\n", tempOctString->data);
        memcpy(outcred->server->data[tempcounter].data, tempOctString->data, ntohl(tempOctString->length));
        /* Step over the counted_octet_string */
        offset = offset + sizeof(uint32_t) + ntohl(tempOctString->length);
    }
    
    /* Done with server parsing */
    tempKeyblock = (keyblock*)offset;
    /* Swap these if it don't work */
    outcred->keyblock.magic = KV5M_KEYBLOCK;
    //outcred->keyblock.enctype = ntohs(tempKeyblock->etype);
    outcred->keyblock.enctype = ntohs(tempKeyblock->keytype);
    DEBUG_PRINT("EncType: %d\n", outcred->keyblock.enctype);
    DEBUG_PRINT("CCache KeyType: %d\n", ntohs(tempKeyblock->keytype));
    outcred->keyblock.length = ntohs(tempKeyblock->keylen);
    DEBUG_PRINT("EncLen: %d\n", outcred->keyblock.length);
    outcred->keyblock.contents = calloc(ntohs(tempKeyblock->keylen) + 1, 1);
    memcpy(outcred->keyblock.contents, tempKeyblock->keyvalue, ntohs(tempKeyblock->keylen));
    offset = offset + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + ntohs(tempKeyblock->keylen);
    tempCredend = (credentialend*)offset;
    outcred->times.authtime = ntohl(tempCredend->time.authtime);
    DEBUG_PRINT("AuthTime: %X\n", outcred->times.authtime);
    outcred->times.starttime = ntohl(tempCredend->time.starttime);
    DEBUG_PRINT("StartTime: %X\n", outcred->times.starttime);
    outcred->times.endtime = ntohl(tempCredend->time.endtime);
    DEBUG_PRINT("EndTime: %X\n", outcred->times.endtime);
    outcred->times.renew_till = ntohl(tempCredend->time.renew_till);
    DEBUG_PRINT("RenewTill: %X\n", outcred->times.renew_till);
    outcred->is_skey = tempCredend->is_skey;
    outcred->ticket_flags = ntohl(tempCredend->tktflags);
    DEBUG_PRINT("Ticket Flags: %X\n", outcred->ticket_flags);
    DEBUG_PRINT("Starting to set the addresses...\n");
    outcred->addresses = calloc(sizeof(krb5_address) * (ntohl(tempCredend->num_address)+1), 1);
    DEBUG_PRINT("Num Address : %d\n", ntohl(tempCredend->num_address));
    offset = (unsigned char*)tempCredend->addrs;
    for (tempcounter = 0; tempcounter < ntohl(tempCredend->num_address); tempcounter++){
        outcred->addresses[tempcounter]->magic = KV5M_ADDRESS;
        outcred->addresses[tempcounter]->addrtype = ntohs(tempCredend->addrs[tempcounter].addrtype);
        DEBUG_PRINT("Addrtype: %d\n", outcred->addresses[tempcounter]->addrtype);
        outcred->addresses[tempcounter]->length = ntohl(tempCredend->addrs[tempcounter].addrdata.length);
        outcred->addresses[tempcounter]->contents = calloc(ntohl(tempCredend->addrs[tempcounter].addrdata.length) + 1, 1);
        memcpy(outcred->addresses[tempcounter]->contents, tempCredend->addrs[tempcounter].addrdata.data, ntohl(tempCredend->addrs[tempcounter].addrdata.length));
        offset = offset + sizeof(uint16_t) + sizeof(uint32_t) + ntohl(tempCredend->addrs[tempcounter].addrdata.length);
    }
    
    /* Done with addresses here */
    /* Start setting authdata */
    tempCredendTwo = (credentialendtwo*)offset;
    outcred->authdata = calloc(sizeof(krb5_authdata) * (ntohl(tempCredendTwo->num_authdata)+1), 1);
    offset = (unsigned char*)tempCredendTwo->authdata;
    DEBUG_PRINT("Num AuthData : %d\n", ntohl(tempCredendTwo->num_authdata));
    for (tempcounter = 0; tempcounter < ntohl(tempCredendTwo->num_authdata); tempcounter++){
        DEBUG_PRINT("Inside loop\n");
        outcred->authdata[tempcounter]->magic = KV5M_AUTHDATA;
        outcred->authdata[tempcounter]->ad_type = ntohs(tempCredendTwo->authdata[tempcounter].authtype);
        outcred->authdata[tempcounter]->length = ntohl(tempCredendTwo->authdata[tempcounter].authdata.length);
        outcred->authdata[tempcounter]->contents = calloc(ntohl(tempCredendTwo->authdata[tempcounter].authdata.length) + 1, 1);
        memcpy(outcred->authdata[tempcounter]->contents, tempCredendTwo->authdata[tempcounter].authdata.data, ntohl(tempCredendTwo->authdata[tempcounter].authdata.length));
        offset = offset + sizeof(uint16_t) + sizeof(uint32_t) + ntohl(tempCredendTwo->authdata[tempcounter].authdata.length);
    }
    
    /* Then do ticket */
    tempOctString = (counted_octet_string*)offset;
    outcred->ticket.magic = KV5M_TICKET;
    outcred->ticket.length = ntohl(tempOctString->length);
    DEBUG_PRINT("TicketLength: %d\n", outcred->ticket.length);
    outcred->ticket.data = calloc(ntohl(tempOctString->length)+1, 1);
    memcpy(outcred->ticket.data, tempOctString->data, ntohl(tempOctString->length));

    return outcred;
}
