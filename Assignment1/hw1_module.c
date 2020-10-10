#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>

#define DIR_NAME "hw1"
#define MAX_PID 131072

MODULE_AUTHOR("Lee, Jisoo");
MODULE_LICENSE("GPL");

static void *hw1_seq_start(struct seq_file *s, loff_t *pos){
	static unsigned long counter = 0;
	if(*pos == 0){
		return &counter;
	}
	else{
		*pos = 0;
		return NULL;
	}
}

static void *hw1_seq_next(struct seq_file *s, void *v, loff_t *pos){
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

static void hw1_seq_stop(struct seq_file *s, void *v){

}

static int hw1_seq_show(struct seq_file *s, void *v){
	loff_t *spos = (loff_t *)v;
	seq_printf(s, "World!\n");

	struct file *fp = s->file;
	const unsigned char *fileName = fp->f_path.dentry->d_name.name;
	seq_printf(s, "%s\n", fileName);

	return 0;
}

/*static int pid_seq_show(struct seq_file *s, void *v){
	loff_t *spos = (loff_t *)v;
	seq_printf(s, "Wow\n");
	return 0;
}*/

static struct seq_operations hw1_seq_ops = {
	.start = hw1_seq_start,
	.next = hw1_seq_next,
	.stop = hw1_seq_stop,
	.show = hw1_seq_show
};

/*static struct seq_operations pid_seq_ops = {
	.start = hw1_seq_start,
	.next = hw1_seq_next,
	.stop = hw1_seq_stop,
	.show = pid_seq_show
};*/

static int hw1_proc_open(struct inode *inode, struct file *file){
	return seq_open(file, &hw1_seq_ops);
}

/*static int pid_proc_open(struct inode *inode, struct file *file){
	return seq_open(file, &pid_seq_ops);
}*/

static const struct file_operations hw1_file_ops = {
	.owner = THIS_MODULE,
	.open = hw1_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*static const struct file_operations pid_file_ops = {
	.owner = THIS_MODULE,
	.open = pid_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};*/

static void __exit hw1_exit(void){
	char pid[100];
	int i;
	remove_proc_entry("top", NULL);
	for(i = 1;i<=MAX_PID;i++){
		sprintf(pid, "%d", i);
		remove_proc_entry(pid, NULL);
	}
	remove_proc_entry(DIR_NAME, NULL);
}

static int __init hw1_init(void){
	char pid[100];
	int i;
	struct proc_dir_entry *proc_dir_entry;
	struct proc_dir_entry *proc_file_entry;
	proc_dir_entry = proc_mkdir(DIR_NAME, NULL);
	proc_file_entry = proc_create("top", 0, proc_dir_entry, &hw1_file_ops);
	for(i = 1;i<=MAX_PID;i++){
		sprintf(pid, "%d", i);
		proc_file_entry = proc_create(pid, 0, proc_dir_entry, &hw1_file_ops);
	}
	return 0;
}

module_init(hw1_init);
module_exit(hw1_exit);