#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#define DRIVER_NAME "pz6_driver"

static const struct pci_device_id pci_wifi_driver_tbl[] = {
	{ PCI_DEVICE(0x8086, 0x51f0) },
	{},
};
MODULE_DEVICE_TABLE(pci, pci_wifi_driver_tbl);

static int pci_driver_probe(struct pci_dev *dev, const struct pci_device_id *ent);
static void pci_driver_remove(struct pci_dev *dev);

static struct pci_driver pci_wifi_driver = {
	.name = DRIVER_NAME,
	.id_table = pci_wifi_driver_tbl,
	.probe = pci_driver_probe,
	.remove = pci_driver_remove,
};
module_pci_driver(pci_wifi_driver);

static int fake_net_dev_open(struct net_device *dev);
static int fake_net_dev_release(struct net_device *dev);
static int fake_net_dev_xmit(struct sk_buff *skb, struct net_device *dev);

const struct net_device_ops fake_net_dev_ops = {
	.ndo_open = fake_net_dev_open,
	.ndo_stop = fake_net_dev_release,
	.ndo_start_xmit = fake_net_dev_xmit,
};


struct net_device *fake_netdev;

static int pci_driver_probe(struct pci_dev *dev, const struct pci_device_id *ent) {
	printk(KERN_INFO DRIVER_NAME ": pci_driver_probe\n");

	int result = 0;
	fake_netdev = alloc_etherdev(0);
	if (!fake_netdev) {
		printk(KERN_ERR DRIVER_NAME ": pci_driver_probe: Failed to alloc_etherdev\n");
		return -ENOMEM;
	}

	fake_netdev->netdev_ops = &fake_net_dev_ops;
	fake_netdev->dev.parent = &dev->dev;

	if ((result = register_netdev(fake_netdev))) {
		printk(KERN_INFO DRIVER_NAME ": pci_driver_probe: Failed to register_netdev: %d\n", result);
		goto err_free_netdev;
	}

	const unsigned long port_addr = pci_resource_start(dev, 0);
	const int len = pci_resource_len(dev, 0);

	u8 *mapped_ptr = ioremap(port_addr, len);

	int j = 0;
	const unsigned char *const ptr = mapped_ptr;
	for (int i = 0; i < len; i++) {
		if (ptr[i] == 0x70) {
			printk(KERN_INFO "offset = %d", i);
			for (j = 0; j < 6; j++) {
				printk(KERN_INFO "MAC %x %x %x %x %x %x \n",
				       (unsigned)ptr[i], (unsigned)ptr[i + 1],
				       (unsigned)ptr[i + 2],
				       (unsigned)ptr[i + 3],
				       (unsigned)ptr[i + 4],
				       (unsigned)ptr[i + 5]);
			}
		}
	}

	iounmap(mapped_ptr);

	return 0;

err_free_netdev:
	free_netdev(fake_netdev);
	fake_netdev = NULL;
	return result;
}

static void pci_driver_remove(struct pci_dev *dev) {
	printk(KERN_INFO DRIVER_NAME ": pci_driver_remove\n");
	if (fake_netdev) {
		unregister_netdev(fake_netdev);
		free_netdev(fake_netdev);
		fake_netdev = NULL;
	}
}

static int fake_net_dev_open(struct net_device *dev) {
	printk(KERN_INFO DRIVER_NAME ": fake_net_dev_open\n");
	return 0;
}

static int fake_net_dev_release(struct net_device *dev) {
	printk(KERN_INFO DRIVER_NAME ": fake_net_dev_release\n");
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


MODULE_DESCRIPTION("MAI drivers training");
MODULE_LICENSE("GPL");
