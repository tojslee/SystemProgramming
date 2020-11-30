#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

#define DIR_NAME "hw2"
#define MAX_PID 131072

MODULE_AUTHOR("Lee, Jisoo");
MODULE_LICENSE("GPL");

int pid;
int period = 10;

module_param(period, int, 0);
module_param(pid, int, 0);


static void *hw2_seq_start(struct seq_file *s, loff_t *pos){
	static unsigned long counter = 0;
	if(*pos == 0){
		return &counter;
	}
	else{
		*pos = 0;
		return NULL;
	}
}

static void *hw2_seq_next(struct seq_file *s, void *v, loff_t *pos){
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

static void hw2_seq_stop(struct seq_file *s, void *v){

}

static void print_bar(struct seq_file *s){
	int i;
	for(i = 0;i<80;++i) seq_printf(s, "-");
	seq_printf(s, "\n");
}

static int hw2_seq_show(struct seq_file *s, void *v){
	loff_t *spos = (loff_t *)v;
	//int ret;

	// get file name
	//struct file *fp = s->file;
	//const unsigned char *fileName = fp->f_path.dentry->d_name.name;

	// check if process is valid
	struct task_struct *p;
	bool flag = false;
	for_each_process(p) {
		if(pid == p->pid){
			flag = true;
			break;
		}
	}

	if(flag){
		if(p->mm == NULL){
			flag = false;
		}
	}


	if(flag){ // valid process
		
	}
	else{ // invlid process
		print_bar(s);
		seq_printf(s, "[System Programming Assingment 2]\n");
		seq_printf(s, "ID: 2018147518, Name: Lee, Jisoo\n");
		seq_printf(s, "There is no information - PID: %d\n", pid);
		print_bar(s);
	}
	

	return 0;
}

static struct seq_operations hw2_seq_ops = {
	.start = hw2_seq_start,
	.next = hw2_seq_next,
	.stop = hw2_seq_stop,
	.show = hw2_seq_show
};

static int hw2_proc_open(struct inode *inode, struct file *file){
	return seq_open(file, &hw2_seq_ops);
}

static const struct file_operations hw2_file_ops = {
	.owner = THIS_MODULE,
	.open = hw2_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

// timer with tasklet
struct timer_list tasklet_timer;
struct tasklet_struct hw2_tasklet;

void tasklet_timer_handler(struct timer_list *tasklet_timer){
	// call tasklet every period
	printk("timer\n");
	tasklet_schedule(&hw2_tasklet);
	// add new timer
	mod_timer(tasklet_timer, jiffies + HZ*period);
}

void tasklet_function(unsigned long data){
	// update data
	printk("tasklet!\n");
}

static int __init hw2_init(void){
	//file creation
	struct proc_dir_entry *proc_file_entry;
	proc_file_entry = proc_create(DIR_NAME, 0, NULL, &hw2_file_ops);
	printk("%d %d\n", period, pid);

	// timer initialization
	timer_setup(&tasklet_timer, tasklet_timer_handler, 0);
	mod_timer(&tasklet_timer, jiffies + HZ*period);

	//tasklet initialization
	tasklet_init(&hw2_tasklet, tasklet_function, 0);

	return 0;
}


static void __exit hw2_exit(void){
	remove_proc_entry(DIR_NAME, NULL);
	del_timer(&tasklet_timer);
	tasklet_kill(&hw2_tasklet);
}

module_init(hw2_init);
module_exit(hw2_exit);
