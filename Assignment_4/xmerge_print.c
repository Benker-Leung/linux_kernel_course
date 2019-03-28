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

        printk(KERN_INFO "the call begins\n");

        /* check the user_space pointers are valid */

        // check args(struct xmerge_param*) from user space is valid or not
        if(!access_ok(VERIFY_WRITE, args, argslen)) {
                printk(KERN_INFO "args is invalid\n");
                return -EFAULT;
        }
        // copy from user to kernel of the (struct xmerge_param)
        if(copy_from_user(&xp, args, argslen)) {
                printk(KERN_INFO "copying args to kernel fail\n");
                return -EFAULT;
        }
        // check char** infiles is valid or not
        if(!access_ok(VERIFY_WRITE, xp.infiles, sizeof(char*)*xp.num_files)) {
                printk(KERN_INFO "infiles is invalid\n");
                return -EFAULT;
        }
        // check int* ofile_count is valid or not
        if(!access_ok(VERIFY_WRITE, xp.ofile_count, sizeof(int))) {
                printk(KERN_INFO "infiles is invalid\n");
                return -EFAULT;
        }
        /* end of checking the user_space pointers */


        /* Before the loop */
        mm_segment_t old_fs;
        old_fs = get_fs();
        set_fs(get_ds());


        // open the file to write if file exists
        fout = ksys_open(xp.outfile, xp.oflags | O_WRONLY, 0);
        // if file not exist, create a new one
        if(fout < 0) {
                fout = ksys_open(xp.outfile, O_CREAT | O_WRONLY, 0);
                // fail to create a file
                if(fout < 0) {
                        printk(KERN_INFO "fail to open fout: [%d]\n", fout);
                        exitCode = fout;
                        goto EXIT;
                }
        }


        /* Write a loop to merge each input files */
        for(i=0; i<xp.num_files; ++i) {

                // get one input file pointer value from user space
                if(copy_from_user(&tempFin, xp.infiles+i, sizeof(char*))) {
                        // entered here if not correctly copy
                        printk(KERN_INFO "copying infiles[%d] fails\n", i);
                        exitCode = -EFAULT;
                        goto CEXIT;
                }
                
                fin = ksys_open(tempFin, O_RDONLY, 0); // open the file to read
                
                if(fin < 0) {
                        // entered if open fails
                        printk(KERN_INFO "fail to open fin: [%d], i:[%d]\n", fin, i);
                        exitCode = fin;
                        goto CEXIT;
                }

                memset(buf, 0, BUFSIZE);        // clear buffer

                // continuous read and write until read nothing
                while((readTemp = ksys_read(fin, buf, BUFSIZE)) > 0) {  // quit if readTemp < 0
                        writeTemp = ksys_write(fout, buf, readTemp);    // record write byte this time
                        // writeTemp shouldn't be less than 0, or error happens
                        if(writeTemp < 0) {
                                printk(KERN_INFO "error during write\n");
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
                printk(KERN_INFO "error during copy to user\n");
                exitCode = -EFAULT;
                goto CEXIT;
        }


CEXIT:  // close fout and exit => CEXIT
        ksys_chmod(xp.outfile, xp.mode);
        ksys_close(fout);
        /* After all file operations, restore the old_fs */

EXIT:   // restore old_fs and exit
        set_fs(old_fs);
        printk(KERN_INFO "exit the syscall xmerge...\n");
        return exitCode;
}
