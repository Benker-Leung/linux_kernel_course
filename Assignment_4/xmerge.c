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
        struct xmerge_param xp;
        __user char* tempFin = NULL;
        char buf[BUFSIZE];
        int writeByte = 0;
        int readTemp = 0;
        int writeTemp = 0;
        int exitCode = 0;
        int i = 0;
        int fin = -1;
        int fout = -1;
        printk(KERN_INFO "the call begins\n");
        /* check the user_space pointers are valid */
        // check args is valid or not
        if(!access_ok(VERIFY_WRITE, args, argslen)) {
                printk(KERN_INFO "args is invalid\n");
                return -EFAULT;
        }
        // copy from user to kernel of the struct
        if(copy_from_user(&xp, args, argslen)) {
                printk(KERN_INFO "copying args to kernel fail\n");
                return -EFAULT;
        }
        // check char** infiles is valid or not
        if(!access_ok(VERIFY_WRITE, xp.infiles, sizeof(char*)*xp.num_files)) {
                printk(KERN_INFO "infiles is invalid\n");
                return -EFAULT;
        }
        /* Before the loop */
        mm_segment_t old_fs;
        old_fs = get_fs();
        set_fs(get_ds());
        /* Write a loop to merge each input files */
        // open the file to write first
        fout = ksys_open(xp.outfile, xp.oflags | O_WRONLY, 0);
        // if file not exist
        if(fout < 0) {
                fout = ksys_open(xp.outfile, O_CREAT | O_WRONLY, 0);
                if(fout < 0) {
                        printk(KERN_INFO "fail to open fout: [%d]\n", fout);
                        exitCode = fout;
                        goto EXIT;
                }
        }
        
        // print each of the charater array address
        for(i=0; i<xp.num_files; ++i) {
                // get one char* of file to read
                if(copy_from_user(&tempFin, xp.infiles+i, sizeof(char*))) {
                        printk(KERN_INFO "copying infiles[%d] fails\n", i);
                        exitCode = -EFAULT;
                        ksys_close(fout);
                        goto EXIT;
                }
                // open the file to read
                fin = ksys_open(tempFin, O_RDONLY, 0);
                if(fin < 0) {
                        printk(KERN_INFO "fail to open fin: [%d], i:[%d]\n", fin, i);
                        exitCode = fin;
                        ksys_close(fout);
                        goto EXIT;
                }
                memset(buf, 0, BUFSIZE);
                // continuous read and write until read nothing
                while((readTemp = ksys_read(fin, buf, BUFSIZE)) > 0) {
                        writeTemp = ksys_write(fout, buf, readTemp);
                        if(writeTemp < 0) {
                                printk(KERN_INFO "error during write\n");
                                exitCode = writeTemp;
                                ksys_close(fout);
                                goto EXIT;
                        }
                        writeByte += writeTemp;
                        memset(buf, 0, BUFSIZE);
                }
                ksys_close(fin);
        }
        ksys_close(fout);
        ksys_chmod(xp.outfile, xp.mode);
        exitCode = writeByte;
        /* After all file operations, restore the old_fs */
 EXIT:
        set_fs(old_fs);
        printk(KERN_INFO "exit the syscall xmerge...\n");
        return exitCode;
}
