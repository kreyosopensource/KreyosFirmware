#include "contiki.h"
#include "CuTest.h"
#include "cfs/cfs.h"
#include <stdio.h>
#include <string.h>

void test_cfs(CuTest* tc)
{
  cfs_coffee_format();

   /*        */
  /* step 1 */
  /*        */
  char message[32];
  char buf[100];
  strcpy(message,"#1.hello world.");
  strcpy(buf,message);

  /* End Step 1. We will add more code below this comment later */    
  /*        */
  /* step 2 */
  /*        */
  /* writing to cfs */
  const char *filename = "/sys/msg_file";
  int fd_write, fd_read;
  int n;
  fd_write = cfs_open(filename, CFS_WRITE);
  if(fd_write != -1) {
    n = cfs_write(fd_write, message, sizeof(message));
    cfs_close(fd_write);
    //printf("step 2: successfully written to cfs. wrote %i bytes\n", n);
  } else {
    CuFail(tc, "ERROR: could not write to memory in step 2.\n");
  } 
  /*        */
  /* step 3 */
  /*        */
  /* reading from cfs */
  strcpy(buf,"empty string");
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read!=-1) {
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 3: %s\n", buf);
    cfs_close(fd_read);
  } else {
    CuFail(tc, "ERROR: could not read from memory in step 3.\n");
  }
  /*        */
  /* step 4 */
  /*        */
  /* adding more data to cfs */
  strcpy(buf,"empty string");
  strcpy(message,"#2.contiki is amazing!");
  fd_write = cfs_open(filename, CFS_WRITE | CFS_APPEND);
  if(fd_write != -1) {
    n = cfs_write(fd_write, message, sizeof(message));
    cfs_close(fd_write);
    printf("step 4: successfully appended data to cfs. wrote %i bytes  \n",n);
  } else {
    CuFail(tc, "ERROR: could not write to memory in step 4.\n");
  }
  /*        */
  /* step 5 */
  /*        */
  /* seeking specific data from cfs */
  strcpy(buf,"empty string");
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read != -1) {
    cfs_read(fd_read, buf, sizeof(message));
    CuAssertTrue(tc, strcmp(buf, "#1.hello world.") == 0);
    printf("step 5: #1 - %s\n", buf);
    cfs_seek(fd_read, sizeof(message), CFS_SEEK_SET);
    cfs_read(fd_read, buf, sizeof(message));
    CuAssertTrue(tc, strcmp(buf, "#2.contiki is amazing!") == 0);
    cfs_close(fd_read);
  } else {
    CuFail(tc, "ERROR: could not read from memory in step 5.\n");
  }
  /*        */
  /* step 6 */
  /*        */
  /* remove the file from cfs */
  cfs_remove(filename);
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read == -1) {
    printf("Successfully removed file\n");
  } else {
    CuFail(tc, "ERROR: could read from memory in step 6.\n");
  }
}


void TestDir(CuTest* tc)
{
  struct cfs_dir dir;
  struct cfs_dirent entry;
  cfs_opendir(&dir, "/");
  while(!cfs_readdir(&dir, &entry))
  {
    printf("entry %s\n", entry.name);
  }

  printf("done\n");
}

CuSuite* cfsGetSuite(void)
{
	CuSuite* suite = CuSuiteNew("cfs");

	SUITE_ADD_TEST(suite, test_cfs);
  SUITE_ADD_TEST(suite, TestDir);


  return suite;
}
