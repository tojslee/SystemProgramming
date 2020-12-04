#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>

#define DIR_NAME "hw1"
#define MAX_PID 131072

MODULE_AUTHOR("Lee, Jisoo");
MODULE_LICENSE("GPL");

extern struct task_struct* sched_history[256][10];
extern u64 last_time[256][10];
extern int last[256];

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

static void print_bar(struct seq_file *s){
	int i;
	for(i = 0;i<40;++i) seq_printf(s, "-");
	seq_printf(s, "\n");
}

static int hw1_seq_show(struct seq_file *s, void *v){
	loff_t *spos = (loff_t *)v;
	int ret;
	const unsigned char *top = "top";
	//seq_printf(s, "World!\n");

	// get file name
	struct file *fp = s->file;
	const unsigned char *fileName = fp->f_path.dentry->d_name.name;
	//seq_printf(s, "%s\n", fileName);

	if(strcmp(fileName, top) == 0){ // if the file name is "top"
		int total_task_num = 0;
		struct task_struct *p;
		int cpu_id = num_online_cpus();
		int i;
		int task;
		int current_task;
		struct task_struct *tcur;
		char *type;

		for_each_process(p) {
			total_task_num += 1;
		}
		//seq_printf(s, "top\n");
		print_bar(s);
		seq_printf(s, "[System Programming Assignment 1]\n");
		seq_printf(s, "ID: 2018147518, Name: Lee, Jisoo\n");
		seq_printf(s, "Total %d tasks, %dHz\n", total_task_num, HZ);
		print_bar(s);
		
		int last_cpu;
		u64 last_t;
		for(i = 0;i<cpu_id;i++){
			seq_printf(s, " CPU %d\n", i);
			print_bar(s);
			last_cpu = last[i];
			for(task = 0;task<10;task++){
				current_task = (last_cpu + task)%10;
				tcur = sched_history[i][current_task];
				if(tcur->prio > 99){type = "CFS";}
				else{type = "RT";}
				last_t = last_time[i][current_task];
				seq_printf(s, "%16s %6d %12lld %3s\n", tcur->comm, tcur->pid, last_t/1000000, type);
			}
			print_bar(s);
		}
	}
	else{ // pid
		//seq_printf(s, "pid\n");
		struct task_struct *p;
		bool flag = false;
		int pid = 0;

		while(*fileName){
			pid = ((*fileName) - '0') + pid*10;
			fileName++;
		}
		//seq_printf(s, "%d\n", pid);

		for_each_process(p) {
			if(pid == p->pid){
				flag = true;
				break;
			}
		}

		if(flag){ // not invalid
			print_bar(s);
			seq_printf(s, "[System Programming Assignment 1]\n");
			seq_printf(s, "ID: 2018147518, Name: Lee, Jisoo\n");
			print_bar(s);

			char *type;
			if(p->prio > 99){type = "CFS";}
			else{type = "RT";}
			seq_printf(s, "Command: %s\nType: %s\nPID :%d\n", p->comm, type, p->pid);
			seq_printf(s, "Start Time: %lld (ms)\n", p->real_start_time/1000000);
			seq_printf(s, "Last Scheduled Time: %lld (ms)\n", p->last_scheduled/1000000);
			// recent_used_cpu in real assignment kernel version
			// no variable named recent_used_cpu in 4.15.0 version kernel
			seq_printf(s, "Last CPU #: %d\nPriority: %d\n", p->recent_used_cpu, p->prio);
			seq_printf(s, "Total Execution Time: %d (ms)\n", p->se.sum_exec_runtime/1000000);

			// for each cpu print exec time
			int cpu_id = num_online_cpus();
			int i;
			for(i = 0;i<cpu_id;i++){
				seq_printf(s, "- CPU %d: %d (ms)\n", i, p->cpu_runtime[i]/1000000);
			}

			if(p->prio > 99){
				seq_printf(s, "Weight: %d\n", p->se.load.weight);
				seq_printf(s, "Virtual Runtime: %d\n", p->se.vruntime/1000000);
			}
		}
	}

	return 0;
}

static struct seq_operations hw1_seq_ops = {
	.start = hw1_seq_start,
	.next = hw1_seq_next,
	.stop = hw1_seq_stop,
	.show = hw1_seq_show
};

static int hw1_proc_open(struct inode *inode, struct file *file){
	return seq_open(file, &hw1_seq_ops);
}

static const struct file_operations hw1_file_ops = {
	.owner = THIS_MODULE,
	.open = hw1_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

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
