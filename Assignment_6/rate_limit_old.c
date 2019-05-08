/* file: flow_net.c 
    Drop packets based on Flow */

/* Step 1: Headers and module setup */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author's name");
MODULE_DESCRIPTION("Drop packets based on Flow");


extern unsigned long volatile jiffies;

// for detect overflow
static unsigned long MAX_ULONG = -1;

/* Step 2: Module parameters and hook operation data structure */
static int port = 80;
MODULE_PARM_DESC(port, "Port number. The default port is 80");
module_param(port, int, 0644);


// static int bytes_allowed = 1<<20;
// MODULE_PARM_DESC(bytes_allowed, "Maximum bytes allowed before dropping. The default value is 1MB");
// module_param(bytes_allowed, int, 0644);
static int rate = 2000;
MODULE_PARM_DESC(rate, "Rate limit in bytes/ms. Default is 2000");
module_param(rate, int, 0644);

static struct nf_hook_ops nfho;

/* Step 3 - Flow struct definition */
struct Flow {
    unsigned int local_ip;
    unsigned int remote_ip;
    unsigned short int local_port;
    unsigned short int remote_port;
    // statistics
    unsigned int pkt_count, drop_count, byte_count, byte_drop_count;
    // record the start jiffies of the first download
    unsigned long volatile start_jiffies;
    int started_download;
};
// FLOW_SIZE = 1<<12 = 4096
#define FLOW_SIZE (1<<12) 

// Create "FLOW_SIZE" buckets to record the flows
static struct Flow flow_list[FLOW_SIZE]; 


/* Step 4: Supporting functions implementation */

// Define a simple hash function for flows
static inline unsigned int hash(struct Flow *f) 
{
    // We can apply any suitable hash algorithm
    // The return value is always in the range of 0..FLOW_SIZE-1
    return ( (f->local_ip%FLOW_SIZE+1) * (f->remote_ip%FLOW_SIZE+1) * 
        ( f->local_port%FLOW_SIZE+1) * (f->remote_port%FLOW_SIZE+1) )% FLOW_SIZE;
}

// An initialization function for a flow
static inline void reset_flow(struct Flow *f) 
{
    if ( f ) { // reset everything to 0
        f->local_port = f->remote_port = 0; 
        f->local_ip = f->remote_ip = 0;     
        f->pkt_count = f->drop_count = 0;   
        f->byte_count = f->byte_drop_count = 0;
        f->start_jiffies = jiffies;
        f->started_download = 0;
    }
}

/* Step 5: The hook function implementation */
static unsigned int hook_func(void *priv, 
                            struct sk_buff *skb, 
                            const struct nf_hook_state *state)
{
    // Define local variables
    struct iphdr *iph;                  // IP header structure
    struct tcphdr *tcph;                // TCP header structure
    unsigned int sip, dip;              // IP addresses
    unsigned int sport, dport;          // Source and Destination port
    struct Flow f;                      // a local flow struct
    struct Flow *fp;                    // a pointer to a flow
    unsigned int payload_len;           // TCP payload length
   
    iph = ip_hdr(skb);                  // Retrieve IP header from skb

    if ( iph->protocol == IPPROTO_TCP ) {
        tcph = tcp_hdr(skb);            // Retrieve TCP header from skb
        sip = iph->saddr;
        dip = iph->daddr;
        sport = ntohs(tcph->source);
        dport = ntohs(tcph->dest);
        if ( sport != port && dport != port )
            return NF_ACCEPT;

        // Calculate the payload length
        payload_len = ntohs(iph->tot_len)-(iph->ihl<<2)-(tcph->doff<<2);

        // Reset a local flow struct
        reset_flow(&f);
        f.local_ip = sip;
        f.remote_ip = dip;
        f.local_port = sport;
        f.remote_port = dport;

        // Use the hash function to find the flow index
        fp = &flow_list[ hash(&f) ];

        if ( tcph->fin ) {

            printk(KERN_INFO "[Finish rate = %d] t = %lu ms, receive / drop(bytes) : %u/%u\n", rate, (jiffies - fp->start_jiffies) * (unsigned long)1000 / (unsigned long)HZ, fp->byte_count, fp->byte_drop_count);

            reset_flow(fp); // end of TCP connection, reset flow
        }


        if ( payload_len <= 0)
            return NF_ACCEPT;   // not a data packet, accept and don't do counting
        
        // initialize start_jiffies if not initialized
        if(!fp->started_download) {
            fp->started_download = 1;
            fp->start_jiffies = jiffies;
        }


        // assumed that the user will stop at reasonable time when overflow of jiffies occur
        // if overflow happen, which is the current jiffies exceed the maximum of unsigned long
        if(time_after(jiffies, MAX_ULONG)) {
            // exceed rate, drop
<<<<<<< HEAD
            if ( (unsigned long)rate * (unsigned long)1000 * (unsigned long)(jiffies + 1 + (MAX_ULONG - fp->start_jiffies)) < ((unsigned long)fp->byte_count + (unsigned long)payload_len) * (unsigned long)HZ ) {
=======
            if ( (unsigned long)rate * (unsigned long)1000 * (unsigned long)(jiffies + (MAX_ULONG - fp->start_jiffies)) < ((unsigned long)fp->byte_count + (unsigned long)payload_len) * (unsigned long)HZ ) {
>>>>>>> a48052a57105777a7807250b9b3e29b868e78373
                fp->drop_count++;
                fp->byte_drop_count += payload_len;
                return NF_DROP;
            } else {
                fp->pkt_count++;
                fp->byte_count += payload_len;
                return NF_ACCEPT;
            }
        }
        // if no overflow happen
        else {
            // exceed rate, drop
            if ( (unsigned long)rate * (unsigned long)1000 * (unsigned long)(jiffies - fp->start_jiffies) < ((unsigned long)fp->byte_count + (unsigned long)payload_len) * (unsigned long)HZ ) {
                fp->drop_count++;
                fp->byte_drop_count += payload_len;
                return NF_DROP;
            } else {
                fp->pkt_count++;
                fp->byte_count += payload_len;
                return NF_ACCEPT;
            }
        }
        
    }
    return NF_ACCEPT; // Accept all other cases
}

int flow_net_init_module(void)
{
    int i;
    for (i=0; i<FLOW_SIZE; i++)
        reset_flow(&flow_list[i]);      // init flows

    nfho.hook = hook_func;              // hook function
    nfho.hooknum = NF_INET_PRE_ROUTING; // pre-routing
    nfho.pf = PF_INET;                  // IPV4 packets
    nfho.priority = NF_IP_PRI_FIRST;	// set to the highest priority

    nf_register_net_hook(&init_net, &nfho); // register hook
    printk(KERN_INFO "[Init] Flow Netfilter Module\n");
    return 0;
}

void flow_net_exit_module(void)
{
    int i;
    struct Flow *fp;
    for (i=0; i<FLOW_SIZE; i++) {
        fp = &flow_list[i];
        if(fp->byte_count != 0) {
            printk(KERN_INFO "[Finish rate = %d] t = %lu ms, receive / drop(bytes) : %u/%u\n", rate, (jiffies - fp->start_jiffies) * (unsigned long)1000 / (unsigned long)HZ, fp->byte_count, fp->byte_drop_count);
        }
    }

    nf_unregister_net_hook(&init_net, &nfho);
    printk(KERN_INFO "[Exit] Flow Netfilter Module\n");
}

module_init(flow_net_init_module);
module_exit(flow_net_exit_module);
