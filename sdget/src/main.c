#include <mgos.h>
#include <mgos_rpc.h>

#include "mgos_sd.h"

static void rpc_sd_open(struct mg_rpc_request_info *ri, void *cb_arg,
                        struct mg_rpc_frame_info *fi, struct mg_str args) {
  struct mgos_sd *sd = mgos_sd_get_global();
  if (NULL != sd) {
    // error
    mg_rpc_send_errorf(ri, 400, "SD already open as %s!",
                       mgos_sd_get_mount_point());
    ri = NULL;
    return;
  }
  //"{sdmmc: %B, mount_point: %Q, format_if_mount_failed: %B}"
  bool sdmmc = false;
  char *mount_point = NULL;
  bool format_if_mount_failed = true;
  json_scanf(args.p, args.len, ri->args_fmt, &sdmmc, &mount_point,
             &format_if_mount_failed);
  if (NULL == mount_point) {
    mg_rpc_send_errorf(ri, 400, "mount_point is missing!");
    return;
  }

  sd = mgos_sd_open(sdmmc, mount_point, format_if_mount_failed);
  if (NULL == sd) {
    mg_rpc_send_errorf(ri, 400, "Failed to open SD (args: %.*s)", args.len,
                       args.p);
    return;
  }
  mg_rpc_send_responsef(ri, "SD created OK!");

  (void) cb_arg;
  (void) fi;
}

static void rpc_sd_close(struct mg_rpc_request_info *ri, void *cb_arg,
                         struct mg_rpc_frame_info *fi, struct mg_str args) {
  struct mgos_sd *sd = mgos_sd_get_global();
  if (NULL == sd) {
    // error
    mg_rpc_send_errorf(ri, 400, "No SD found!");
    return;
  }
  mgos_sd_close();
  mg_rpc_send_responsef(ri, "SD closed.");

  (void) cb_arg;
  (void) fi;
  (void) args;
}




static void rpc_sd_get(struct mg_rpc_request_info *ri, void *cb_arg,
                               struct mg_rpc_frame_info *fi,
                               struct mg_str args){
     char *filename = NULL;
     long offset = 0, len = -1;
     long file_size = 0;
     FILE *fp = NULL;
     char *data = NULL;

     json_scanf(args.p, args.len, ri->args_fmt, &filename, &offset, &len);

     if (filename == NULL) {
    mg_rpc_send_errorf(ri, 400, "filename is required");
    ri = NULL;
    goto clean;
  }

  if (offset < 0) {
    mg_rpc_send_errorf(ri, 400, "illegal offset");
    ri = NULL;
    goto clean;
  }

  /* try to open file */
  fp = fopen(filename, "rb");

  if (fp == NULL) {
    mg_rpc_send_errorf(ri, 400, "failed to open file \"%s\"", filename);
    ri = NULL;
    goto clean;
  }

  /* determine file size */
  if (fseek(fp, 0, SEEK_END) != 0) {
    mg_rpc_send_errorf(ri, 500, "fseek");
    ri = NULL;
    goto clean;
  }

  file_size = (long) ftell(fp);
  if (file_size < 0) {
    mg_rpc_send_errorf(ri, 500, "ftell");
    ri = NULL;
    goto clean;
  }

  /* determine the size of the chunk to read */
  if (offset > file_size) {
    offset = file_size;
  }
  if (len < 0 || offset + len > file_size) {
    len = file_size - offset;
  }

  if (len > 0) {
    /* try to allocate the chunk of needed size */
    data = (char *) malloc(len);
    if (data == NULL) {
      mg_rpc_send_errorf(ri, 500, "out of memory");
      ri = NULL;
      goto clean;
    }

    if (offset == 0) {
      LOG(LL_INFO, ("Sending %s", filename));
    }

    /* seek & read the data */
    if (fseek(fp, offset, SEEK_SET)) {
      mg_rpc_send_errorf(ri, 500, "fseek");
      ri = NULL;
      goto clean;
    }

    if ((long) fread(data, 1, len, fp) != len) {
      mg_rpc_send_errorf(ri, 500, "fread");
      ri = NULL;
      goto clean;
    }
  }

  /* send the response */
  mg_rpc_send_responsef(ri, "{data: %V, left: %d}", data, len,
                        (file_size - offset - len));
  ri = NULL;

clean:
  if (filename != NULL) {
    free(filename);
  }

  if (data != NULL) {
    free(data);
  }

  if (fp != NULL) {
    fclose(fp);
  }

  (void) cb_arg;
  (void) fi;
}

struct put_data {
  char *p;
  int len;
};





                               }








bool mgos_rpc_service_sd_init() {
  struct mg_rpc *c = mgos_rpc_get_global();
  mg_rpc_add_handler(c, "SD.Open",
                     "{sdmmc: %B, mount_point: %Q, format_if_mount_failed: %B}",
                     rpc_sd_open, NULL);
  mg_rpc_add_handler(c, "SD.Close", "", rpc_sd_close, NULL);
 
  
  
  mg_rpc_add_handler(c, "SD.Get", "{filename: %Q, offset: %ld, len: %ld}",
                     rpc_sd_get_handler, NULL);
  

  
 

  return true;
}


