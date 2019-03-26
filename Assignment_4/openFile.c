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
        __user char* temp = NULL;
        int i;
        int fd;

        printk(KERN_INFO "the call begins\n");

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

        // args and struct values
        printk(KERN_INFO "args: [%u]\n", args);
        printk(KERN_INFO "outfile: [%u]\n", xp.outfile);
        printk(KERN_INFO "infiles: [%u]\n", xp.infiles);
        printk(KERN_INFO "num_files: [%u]\n", xp.num_files);
        printk(KERN_INFO "oflags: [%u]\n", xp.oflags);
        printk(KERN_INFO "mode: [%u]\n", xp.mode);
        printk(KERN_INFO "ofile_count: [%u]\n", xp.ofile_count);

        // check char** infiles is valid or not
        if(!access_ok(VERIFY_WRITE, xp.infiles, sizeof(char*)*xp.num_files)) {
                printk(KERN_INFO "infiles is invalid\n");
                return -EFAULT;
        }

        printk(KERN_INFO "sizeof char start [%u]\n", sizeof(char*));
        printk(KERN_INFO "sizeof char start [%u]\n", sizeof(__user char*));
        printk(KERN_INFO "sizeof char start [%u]\n", sizeof(char**));

        // print each of the charater array address
        for(i=0; i<xp.num_files; ++i) {

                printk(KERN_INFO "deref:[%u]\n", xp.infiles);
                // get value of char*
                if(copy_from_user(&temp, xp.infiles+i, sizeof(char*))) {
                        printk(KERN_INFO, "copying infiles[%d] fails\n", i);
                        return -EFAULT;
                }
                printk(KERN_INFO "infiles[%d]: [%u]\n", i, temp);

                // open the file, assume exists
                fd = ksys_open(temp, O_RDWR, 0);
                if(fd < 0) {
                        printk(KERN_INFO "cannot open file [%d]\n", i);
                }
                else {
                        printk(KERN_INFO "can open file [%d]\n", i);
                        ksys_close(fd);
                }
        }


        /* Before the loop */
        mm_segment_t old_fs;
        old_fs = get_fs();
        set_fs(get_ds());

        /* Write a loop to merge each input files */

        /* After all file operations, restore the old_fs */
        set_fs(old_fs);

        printk(KERN_INFO "seems no error for this call\n");
        return 0;

}
