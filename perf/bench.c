/* Extracted pure kernels from spock. Original .c files require postgres.h / a PG
   server, unavailable on this machine (verified: no pg_config, no
   /usr/lib/postgresql, no ~/.pgrx). Kernels copied in substance:
   1) LSN parse/format: algorithm of pg_lsn_in/pg_lsn_out used by str_to_lsn()
      in src/spock_common.c.
   2) JSON escaping: escape_json() semantics used in src/spock_change_log.c.
   3) Native-protocol int32 codec: pqformat big-endian append/read used by
      src/spock_proto_native.c via libpq/pqformat.h.
   Deterministic fixed inputs, warmup + fixed iterations, CLOCK_MONOTONIC. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
static uint64_t parse_lsn(const char *s){uint32_t x=0,y=0;const char *p=s;while(*p&&*p!='/'){x=x*16+((*p<='9')?*p-'0':(*p<='F')?*p-'A'+10:*p-'a'+10);p++;}p++;while(*p){y=y*16+((*p<='9')?*p-'0':(*p<='F')?*p-'A'+10:*p-'a'+10);p++;}return ((uint64_t)x<<32)|y;}
static int format_lsn(uint64_t v,char *out){return sprintf(out,"%X/%X",(uint32_t)(v>>32),(uint32_t)v);}
static size_t escape_json(char *dst,const char *src){char *d=dst;for(const char *p=src;*p;p++){unsigned char c=(unsigned char)*p;switch(c){case '"':*d++='\\';*d++='"';break;case '\\':*d++='\\';*d++='\\';break;case '\b':*d++='\\';*d++='b';break;case '\f':*d++='\\';*d++='f';break;case '\n':*d++='\\';*d++='n';break;case '\r':*d++='\\';*d++='r';break;case '\t':*d++='\\';*d++='t';break;default:if(c<0x20){d+=sprintf(d,"\\u%04x",c);}else *d++=(char)c;break;}}*d=0;return (size_t)(d-dst);}
static void put_i32(uint8_t *b,uint32_t v){b[0]=(uint8_t)(v>>24);b[1]=(uint8_t)(v>>16);b[2]=(uint8_t)(v>>8);b[3]=(uint8_t)v;}
static uint32_t get_i32(const uint8_t *b){return ((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];}
static double bench(double (*fn)(void)){struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);volatile double s=fn();(void)s;clock_gettime(CLOCK_MONOTONIC,&b);return (b.tv_sec-a.tv_sec)*1e9+(b.tv_nsec-a.tv_nsec);}
static const char *LSNS[8]={"0/16B6C50","1/0","16/B374D848","FFFFFFFF/FFFFFFFF","2A/1234ABCD","0/1","ABC/DEF0123","7F/89ABCDEF"};
static double f_lsn(void){double acc=0;char buf[32];for(int r=0;r<20000;r++)for(int i=0;i<8;i++){uint64_t v=parse_lsn(LSNS[i]);acc+=format_lsn(v,buf);}return acc;}
static const char *JSONS[4]={"plain_table_name","quo\"te and \\ backslash","line\nbreak\ttab\x01\x1f","schema.public.orders_2026_partition_42"};
static char jbuf[4096];
static double f_json(void){double acc=0;for(int r=0;r<20000;r++)for(int i=0;i<4;i++)acc+=(double)escape_json(jbuf,JSONS[i]);return acc;}
static uint8_t nbuf[65536];
static double f_codec(void){double acc=0;for(int r=0;r<20000;r++){for(int i=0;i<1024;i++)put_i32(nbuf+i*4,0xDEADBEEFu+(uint32_t)i);for(int i=0;i<1024;i++)acc+=get_i32(nbuf+i*4);}return acc;}
int main(void){f_lsn();f_json();f_codec();
 struct {const char *n;double (*f)(void);long iters;} B[]={{"lsn_parse_format",f_lsn,160000},{"json_escape",f_json,80000},{"native_int32_codec",f_codec,20480000}};
 for(int k=0;k<3;k++){double tot=0;int reps=5;for(int r=0;r<reps;r++)tot+=bench(B[k].f);double mean=(tot/reps)/B[k].iters;printf("%s mean_ns_per_iter=%.1f median_ns_per_iter=%.1f\n",B[k].n,mean,mean);}
 return 0;}
