#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

#define BUFSIZE 256
struct xmerge_param {
        __user char *outfile;
        __user char **infiles;
        unsigned int num_files;
        int oflags;
        mode_t mode;
        __user int *ofile_count;
};
SYSCALL_DEFINE2(xmerge, void*, args, size_t, argslen)
{
        /* Local variable of the xmerge syscall */
        struct xmerge_param xp;         // used for store user space param
        __user char* tempFin = NULL;    // used for temp store of input file pointer in user space
        char buf[BUFSIZE];              // used to store char read
        int writeByte = 0;              // count the write byte in total
        int readTemp = 0;               // count the read byte each time
        int writeTemp = 0;              // count the write byte each time
        int exitCode = 0;               // exit integer return to user space program
        int i = 0;                      // used for counting
        int fin = -1;                   // used for file descriptor for input file
        int fout = -1;                  // used for file descriptor for output file
        int ofileCount = 0;             // used for counting merged number of file


        /* check the user_space pointers are valid */

        // check args(struct xmerge_param*) from user space is valid or not
        if(!access_ok(VERIFY_WRITE, args, argslen)) {
                //args is invalid
                return -EFAULT;
        }
        // copy from user to kernel of the (struct xmerge_param)
        if(copy_from_user(&xp, args, argslen)) {
                //copying args to kernel fail
                return -EFAULT;
        }
        // check char** infiles is valid or not
        if(!access_ok(VERIFY_WRITE, xp.infiles, sizeof(char*)*xp.num_files)) {
                //infiles is invalid
                return -EFAULT;
        }
        // check int* ofile_count is valid or not
        if(!access_ok(VERIFY_WRITE, xp.ofile_count, sizeof(int))) {
                //infiles is invalid
                return -EFAULT;
        }
        /* end of checking the user_space pointers */


        /* Before the loop */
        mm_segment_t old_fs;
        old_fs = get_fs();
        set_fs(get_ds());


        // open the file to write if file exists
        fout = ksys_open(xp.outfile, xp.oflags | O_WRONLY, 0);
        if(fout < 0) {
                // fail to open file
                exitCode = fout;
                goto EXIT;
        }


        /* Write a loop to merge each input files */
        for(i=0; i<xp.num_files; ++i) {

                // get one input file pointer value from user space
                if(copy_from_user(&tempFin, xp.infiles+i, sizeof(char*))) {
                        // entered here if not correctly copy
                        //copying infiles
                        exitCode = -EFAULT;
                        goto CEXIT;
                }
                
                fin = ksys_open(tempFin, O_RDONLY, 0); // open the file to read
                
                if(fin < 0) {
                        // entered if open fails
                        //fail to open fin
                        exitCode = fin;
                        goto CEXIT;
                }

                memset(buf, 0, BUFSIZE);        // clear buffer

                // continuous read and write until read nothing
                while((readTemp = ksys_read(fin, buf, BUFSIZE)) > 0) {  // quit if readTemp < 0
                        writeTemp = ksys_write(fout, buf, readTemp);    // record write byte this time
                        // writeTemp shouldn't be less than 0, or error happens
                        if(writeTemp < 0) {
                                //error during write
                                exitCode = writeTemp;
                                goto CEXIT;
                        }
                        writeByte += writeTemp;         // count the total bytes wrote
                        memset(buf, 0, BUFSIZE);
                }
                ++ofileCount;           // increment merge file num
                ksys_close(fin);        // close the file after merge
        }
        
        // maybe no error, then return bytes wrote to user
        exitCode = writeByte;

        // copy the ofile_count back to user
        if(copy_to_user(xp.ofile_count, &ofileCount, sizeof(int))) {
                //error during copy to user
                exitCode = -EFAULT;
                goto CEXIT;
        }


CEXIT:  // close fout and exit => CEXIT
        ksys_chmod(xp.outfile, xp.mode);
        ksys_close(fout);
        /* After all file operations, restore the old_fs */

EXIT:   // restore old_fs and exit
        set_fs(old_fs);
        //exit the syscall xmerge...
        return exitCode;
}
