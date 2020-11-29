#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/giffies.h>

#define DIR_NAME "hw2"
#define MAX_PID 131072

MODULE_AUTHOR("Lee, Jisoo");
MODULE_LICENSE("GPL");

static int pid = 2;
static int period = 10;

//module_param(period, int, 0);
//module_param(pid, int, 0);


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
	int ret;

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

static void __exit hw2_exit(void){
	char pids[100];
	int i;
	for(i = 1;i<=MAX_PID;i++){
		sprintf(pids, "%d", i);
		remove_proc_entry(pids, NULL);
	}
	remove_proc_entry(DIR_NAME, NULL);
}

static int __init hw2_init(void){
	char pids[100];
	//int i;
	struct proc_dir_entry *proc_dir_entry;
	struct proc_dir_entry *proc_file_entry;
	proc_dir_entry = proc_mkdir(DIR_NAME, NULL);
	/*for(i = 1;i<=MAX_PID;i++){
		sprintf(pids, "%d", i);
		proc_file_entry = proc_create(pids, 0, proc_dir_entry, &hw2_file_ops);
	}*/
	sprintf(pids, "%d", pid);
	proc_file_entry = proc_create(pids, 0, proc_dir_entry, &hw2_file_ops);
	return 0;
}

module_init(hw2_init);
module_exit(hw2_exit);

// tasklet updating data


/*void tasklet_function(unsigned long data){
	// update data
	printk("tasklet!\n");
}

DECLARE_TASKLET(hw2_tasklet, tasklet_function, pid);*/

// timer with tasklet
struct timer_list tasklet_timer;


void tasklet_timer_handler(unsigned long arg){
	// call tasklet every period
	//tasklet_schedule(&hw2_tasklet);
	printk("timer\n");
	// add new timer
	mod_timer(&tasklet_timer, jiffies + HZ*period);
}

void tasklet_timer_init(){
	init_timer(&tasklet_timer);
	tasklet_timer.functions = tasklet_timer_handler;
	tasklet_timer.expires = jiffies + HZ*period;

	add_timer(&tasklet_timer);
}


//del_timer(&tasklet_timer);


