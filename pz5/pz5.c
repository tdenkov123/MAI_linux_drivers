#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

#define DRIVER_NAME "pz5_driver"

static __init int init(void);
static __exit void cleanup(void);

struct net_device *demo_nic;

static int demo_nic_open(struct net_device *dev) {
	printk(KERN_INFO DRIVER_NAME ": open\n");
	return 0;
}

static int demo_nic_release(struct net_device *dev) {
	printk(KERN_INFO DRIVER_NAME ": release\n");
	netif_stop_queue(dev);
	return 0;
}

static int fake_net_dev_xmit(struct sk_buff *skb, struct net_device *dev) {
	printk(KERN_INFO DRIVER_NAME ": xmit\n");

	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets++;

	printk(KERN_INFO DRIVER_NAME ": %pTN<struct sk_buff>\n", (void *)skb);
	printk(KERN_INFO DRIVER_NAME
		": Packet info - Len: %u, Head: %p, Data: %p, Tail: %d, End: %d\n",
		skb->len, (void *)skb->head, (void *)skb->data, skb->tail,
		skb->end);

	printk(KERN_INFO DRIVER_NAME ": First %d bytes:\n", skb->len);
	printk(KERN_INFO DRIVER_NAME ": Hex: ");
	for (uint i = 0; i < skb->len; i++) {
		pr_cont("%02x ", skb->data[i]);
	}
	pr_cont("\n");

	printk(KERN_INFO DRIVER_NAME ": Hex: ");
	for (uint i = 0; i < skb->len; i++) {
		pr_cont("%c ", skb->data[i]);
	}
	pr_cont("\n");

	dev_kfree_skb(skb);
	return 0;
}

static int demo_nic_init(struct net_device *dev) {
	printk(KERN_INFO DRIVER_NAME ": initialized\n");

	return 0;
};

const struct net_device_ops fake_net_device_ops = {
	.ndo_init = demo_nic_init,
	.ndo_open = demo_nic_open,
	.ndo_stop = demo_nic_release,
	.ndo_start_xmit = fake_net_dev_xmit,
};

static __init int init(void) {
	int result;

	demo_nic = alloc_etherdev(0);
	demo_nic->netdev_ops = &fake_net_device_ops;

	if ((result = register_netdev(demo_nic))) {
		printk(KERN_INFO DRIVER_NAME ": Error %d initalizing card ...", result);
		return result;
	}
	return 0;
}

static __exit void cleanup(void) {
	printk(KERN_INFO DRIVER_NAME ": Cleaning Up the Module\n");
	unregister_netdev(demo_nic);
	free_netdev(demo_nic);
}


module_init(init);
module_exit(cleanup);

MODULE_DESCRIPTION("MAI drivers training");
MODULE_LICENSE("GPL");
