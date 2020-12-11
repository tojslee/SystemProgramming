#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#define DIR_NAME "hw2"
#define MAX_PID 131072

MODULE_AUTHOR("Lee, Jisoo");
MODULE_LICENSE("GPL");

int pid;
int period = 10;
int idx = 1;

// for synchronization
spinlock_t tasklet_lock;
DEFINE_SPINLOCK(tasklet_lock);

// for information of the process (tasklet update)
char *comm;
static long page_size = PAGE_SIZE;
pgd_t* pgd_base;
int leng;
long last_update_time;
struct vm_area* mmap;

struct vm_area{
	char read[10];
	char write[10];
	char exec[10];
	char type[30];
	unsigned long vm_start, vm_end;
	unsigned long pm_start, pm_end;
	int pageNum;
	struct vm_area* next;
};


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
	bool flag = false;
	if(last_update_time == -1){
		flag = false;
	}
	else{flag = true;}
	spin_lock(&tasklet_lock);

	if(flag){ // valid process
		print_bar(s);
		seq_printf(s, "[System Programming Assignment 2]\n");
		seq_printf(s, "ID: 2018147518, Name: Lee, Jisoo\n");
		seq_printf(s, "Command: %s, PID: %d\n", comm, pid);
		print_bar(s);

		seq_printf(s, "Last update time: %llu ms\n", last_update_time);
		seq_printf(s, "Page Size: %d KB\n", page_size);
		seq_printf(s, "PGD Base Address: 0x%08lx\n", pgd_base);
		//seq_printf(s, "temp: %d\n", temp);
		print_bar(s);

		
		int index = 1;
		struct vm_area* te = mmap;
		while(index <= leng){
			seq_printf(s, "VM Area $%d - %s, (%d pages)\n", index++, te->type, te->pageNum);
			seq_printf(s, "Permissions: R(%s), W(%s), X(%s)\n", te->read, te->write, te->exec);
			seq_printf(s, "Virtual Memory: 0x%08lx - 0x%08lx\n", te->vm_start, te->vm_end);
			seq_printf(s, "Physical Memory: 0x%08lx - 0x%08lx\n", te->pm_start, te->pm_end);
			print_bar(s);
			te = te->next;
		}

	}
	else{ // invlid process
		print_bar(s);
		seq_printf(s, "[System Programming Assingment 2]\n");
		seq_printf(s, "ID: 2018147518, Name: Lee, Jisoo\n");
		seq_printf(s, "There is no information - PID: %d\n", pid);
		print_bar(s);
	}
	spin_unlock(&tasklet_lock);
	

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
	tasklet_schedule(&hw2_tasklet);
	// add new timer
	mod_timer(tasklet_timer, jiffies + HZ*period);
}

void tasklet_function(unsigned long data){
	// update data
	
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

	// for synchronization of informatioin updating in tasklet
	struct timespec current_time;
	spin_lock(&tasklet_lock);
	leng = 0;
	if(flag){ // existing process
		comm = p->comm;
		pgd_base = p->mm->pgd;
		getnstimeofday(&current_time);
		last_update_time = current_time.tv_sec*1000+current_time.tv_nsec/1000000;
		
		
		struct vm_area_struct *fos = p->mm->mmap;
		struct vm_area *tp;
		while(mmap != NULL){
			tp = mmap;
			mmap = mmap->next;
			kfree(tp);
		}
		mmap = kmalloc(sizeof(struct vm_area), GFP_KERNEL);
		mmap->next = NULL;
		struct vm_area* cur = mmap;
		while(fos != NULL){
			leng++;
			cur->vm_start = fos->vm_start;
			cur->vm_end = fos->vm_end;
			cur->pageNum = p->mm->total_vm;
			
			// area
			if(p->mm->start_code <= fos->vm_start && p->mm->end_code >= fos->vm_end){
				// TEXT area
				//codes[temp] = "Code";
				strcpy(cur->type, "Code");
			}
			else if(p->mm->start_data <= fos->vm_start && p->mm->end_data >= fos->vm_end){
				// DATA area
				//codes[temp] = "Data";
				strcpy(cur->type,"Data");
			}
			else if(p->mm->end_data < fos->vm_start && p->mm->start_brk > fos->vm_end){
				// BSS
				//codes[temp] = "BSS";
				strcpy(cur->type,"BSS");
			}
			else if(p->mm->start_brk <= fos->vm_start && p->mm->brk >= fos->vm_end){
				// heap
				//codes[temp] = "Heap";
				strcpy(cur->type,"Heap");
			}
			else if(p->mm->start_stack >= fos->vm_start && p->mm->start_stack <= fos->vm_end){
				// stack
				//codes[temp] = "Stack";
				strcpy(cur->type,"Stack");
			}
			else{
				//shared
				//codes[temp] = "Shared Libraries";
				strcpy(cur->type,"Shared Libraries");
			}

			// access
			int flags = fos->vm_flags;
			if(flags & VM_READ){
				strcpy(cur->read,"yes");
			}
			else{strcpy(cur->read,"no");}
			if (flags & VM_WRITE){
				strcpy(cur->write,"yes");
			}
			else{strcpy(cur->write,"no");}
			if(flags & VM_EXEC){
				strcpy(cur->exec,"yes");
			}
			else{strcpy(cur->exec,"no");}
			//temp++;
			pgd_t *pgd;
			p4d_t *p4d;
			pud_t *pud;
			pmd_t *pmd;
			pte_t *pte;

			pgd = pgd_offset(p->mm, fos->vm_start);
			p4d = p4d_offset(pgd, fos->vm_start);
			pud = pud_offset(p4d, fos->vm_start);
			pmd = pmd_offset(pud, fos->vm_start);
			pte = pte_offset_kernel(pmd, fos->vm_start);
			cur->pm_start = pte_page(*pte);

			pgd = pgd_offset(p->mm, fos->vm_end);
			p4d = p4d_offset(pgd, fos->vm_end);
			pud = pud_offset(p4d, fos->vm_end);
			pmd = pmd_offset(pud, fos->vm_end);
			pte = pte_offset_kernel(pmd, fos->vm_start);
			cur->pm_end = pte_page(*pte);

			fos = fos->vm_next;
			cur->next = kmalloc(sizeof(struct vm_area), GFP_KERNEL);
			cur = cur->next;
			cur->next = NULL;
		}
		cur = NULL;
	}
	else{ // else
		last_update_time = -1;
	}
	spin_unlock(&tasklet_lock);
}

static int __init hw2_init(void){
	//file creation
	struct proc_dir_entry *proc_file_entry;
	proc_file_entry = proc_create(DIR_NAME, 0, NULL, &hw2_file_ops);

	mmap = kmalloc(sizeof(struct vm_area), GFP_KERNEL);
	mmap->next = NULL;

	// timer initialization
	timer_setup(&tasklet_timer, tasklet_timer_handler, 0);
	mod_timer(&tasklet_timer, jiffies + HZ*period);

	//tasklet initialization
	tasklet_init(&hw2_tasklet, tasklet_function, 0);
	tasklet_schedule(&hw2_tasklet);

	return 0;
}


static void __exit hw2_exit(void){
	remove_proc_entry(DIR_NAME, NULL);
	del_timer(&tasklet_timer);
	tasklet_kill(&hw2_tasklet);
	struct vm_area* tp;
	while(mmap != NULL){
		tp = mmap;
		mmap = mmap->next;
		kfree(tp);
	}
}

module_init(hw2_init);
module_exit(hw2_exit);
