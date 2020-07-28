/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <zephyr.h>
#include <arch/cpu.h>
#include <sys/byteorder.h>
#include <logging/log.h>
#include <sys/util.h>
#include <drivers/ipm.h>

#include <openamp/open_amp.h>
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/alloc.h>

//#include <net/buf.h>

#include <event_manager.h>

#define MODULE main
#include "module_state_event.h"
#include "key_id.h"
#define LOG_LEVEL LOG_LEVEL_INFO

#define LOG_MODULE_NAME dualcore_net
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* Configuration defines */
#if !DT_HAS_CHOSEN(zephyr_ipc_shm)
#error "Sample requires definition of shared memory for rpmsg"
#endif

#define SHM_NODE            DT_CHOSEN(zephyr_ipc_shm)
#define SHM_BASE_ADDRESS    DT_REG_ADDR(SHM_NODE)
#define SHM_START_ADDR      (SHM_BASE_ADDRESS + 0x400)
#define SHM_SIZE            0x7c00
#define SHM_DEVICE_NAME     "sram0.shm"

BUILD_ASSERT((SHM_START_ADDR + SHM_SIZE - SHM_BASE_ADDRESS)
		<= DT_REG_SIZE(SHM_NODE),
	"Allocated size exceeds available shared memory reserved for IPC");

#define VRING_COUNT         2
#define VRING_TX_ADDRESS    (SHM_START_ADDR + SHM_SIZE - 0x400)
#define VRING_RX_ADDRESS    (VRING_TX_ADDRESS - 0x400)
#define VRING_ALIGNMENT     4
#define VRING_SIZE          16

#define VDEV_STATUS_ADDR    SHM_BASE_ADDRESS

/* End of configuration defines */

//static bool ready_to_transfer = false;
//static bool app_ready_to_send = false;
extern void send_button_event(u16_t key_id, bool pressed);

struct button_struct_t {
	u16_t key_id;
};

struct button_struct_t button_struct;

static struct device *ipm_tx_handle;
static struct device *ipm_rx_handle;

static metal_phys_addr_t shm_physmap[] = { SHM_START_ADDR };
static struct metal_device shm_device = {
	.name = SHM_DEVICE_NAME,
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt       = (void *) SHM_START_ADDR,
			.physmap    = shm_physmap,
			.size       = SHM_SIZE,
			.page_shift = 0xffffffff,
			.page_mask  = 0xffffffff,
			.mem_flags  = 0,
			.ops        = { NULL },
		},
	},
	.node = { NULL },
	.irq_num = 0,
	.irq_info = NULL
};

static struct virtqueue *vq[2];
static struct rpmsg_endpoint ep;

static struct k_work ipm_work;

static unsigned char virtio_get_status(struct virtio_device *vdev)
{
	return sys_read8(VDEV_STATUS_ADDR);
}

static u32_t virtio_get_features(struct virtio_device *vdev)
{
	return BIT(VIRTIO_RPMSG_F_NS);
}

static void virtio_set_status(struct virtio_device *vdev, unsigned char status)
{
	sys_write8(status, VDEV_STATUS_ADDR);
}

static void virtio_notify(struct virtqueue *vq)
{
	int status;

	status = ipm_send(ipm_tx_handle, 0, 0, NULL, 0);
	if (status != 0) {
		LOG_ERR("ipm_send failed to notify: %d", status);
	}
}

const struct virtio_dispatch dispatch = {
	.get_status = virtio_get_status,
	.set_status = virtio_set_status,
	.get_features = virtio_get_features,
	.notify = virtio_notify,
};

static void ipm_callback_process(struct k_work *work)
{
	virtqueue_notification(vq[1]);
}

static void ipm_callback(void *context, u32_t id, volatile void *data)
{
	LOG_INF("Got callback of id %u", id);
	k_work_submit(&ipm_work);
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
	rpmsg_destroy_ept(ep);
}

static int send_data(void *data, size_t len)
{
	LOG_INF("Sending data...");
	rpmsg_send(&ep, data, len);
	return 0;
}

// u16_t int_to_keyid(int c)
// {
// 	switch (c)
// 	{
// 	case 72: //  H
// 		return KEY_ID(0x00, 0x0B);
// 		break;
// 	case 101: // e
// 		return KEY_ID(0x00, 0x08);
// 		break;
// 	case 108: // l
// 		return KEY_ID(0x00, 0x0F);
// 		break;
// 	case 111: // o
// 		return KEY_ID(0x00, 0x12);
// 		break;
// 	case 32: //  space
// 		return KEY_ID(0x00, 0x2C);
// 		break;
// 	case 87: //  W
// 		return KEY_ID(0x00, 0x1A);
// 		break;
// 	case 114: // r
// 		return KEY_ID(0x00, 0x15);
// 		break;
// 	case 100: // d
// 		return KEY_ID(0x00, 0x07);
// 		break;

// 	default:
// 		LOG_ERR("ERROR: Tried to convert unsupported key!");
// 		return NULL;
// 		break;
// 	}
// }

// void set_ready_to_transfer(void)
// {
// 	if(app_ready_to_send)
// 	{
// 		uint8_t msg[15] = "Hello World ";
// 		msg[14] = 0;
	
// 		send_data(msg, sizeof(msg));
// 		ready_to_transfer = true;
// 	}
// }

int endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, u32_t src,
		void *priv)
{
	//LOG_INF("Received message of %u bytes.", len);
	//LOG_HEXDUMP_DBG((uint8_t *)data, len, "Data:");
	//LOG_INF("Data: \n\n%s\n", log_strdup((uint8_t *)data));

	//hci_rpmsg_rx((u8_t *) data, len);

	//int int_buffr = 0;
	//memcpy(&int_buffr, (uint8_t*)data, sizeof(uint8_t));
	// if(!app_ready_to_send)
	// {
	// 	LOG_INF("Data: \n\n%s\n", log_strdup((uint8_t *)data));
	// 	//send_data(data, len);
	// 	app_ready_to_send = true;
	// }
	// else if(ready_to_transfer)
	if (data)
	{
		memcpy(&button_struct, (struct button_struct_t*)data, sizeof(struct button_struct_t));
		//LOG_INF("\nchar: %s \nint: %d", log_strdup((uint8_t *)data), int_buffr);
		//u16_t key_id = int_to_keyid(int_buffr);
		send_button_event(button_struct.key_id, true);
		send_button_event(button_struct.key_id, false);
	}
	if (!data)
	{
		LOG_ERR("Empty message received.");
	}
	
	return RPMSG_SUCCESS;
}

static int hci_rpmsg_init(void)
{
	int err;
	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;

	static struct virtio_vring_info   rvrings[2];
	static struct virtio_device       vdev;
	static struct rpmsg_device        *rdev;
	static struct rpmsg_virtio_device rvdev;
	static struct metal_io_region     *io;
	static struct metal_device        *device;

	/* Setup IPM workqueue item */
	k_work_init(&ipm_work, ipm_callback_process);

	/* Libmetal setup */
	err = metal_init(&metal_params);
	if (err) {
		LOG_ERR("metal_init: failed - error code %d", err);
		return err;
	}

	err = metal_register_generic_device(&shm_device);
	if (err) {
		LOG_ERR("Couldn't register shared memory device: %d", err);
		return err;
	}

	err = metal_device_open("generic", SHM_DEVICE_NAME, &device);
	if (err) {
		LOG_ERR("metal_device_open failed: %d", err);
		return err;
	}

	io = metal_device_io_region(device, 0);
	if (!io) {
		LOG_ERR("metal_device_io_region failed to get region");
		return -ENODEV;
	}

	/* IPM setup */
	ipm_tx_handle = device_get_binding("IPM_1");
	if (!ipm_tx_handle) {
		LOG_ERR("Could not get TX IPM device handle");
		return -ENODEV;
	}

	ipm_rx_handle = device_get_binding("IPM_0");
	if (!ipm_rx_handle) {
		LOG_ERR("Could not get RX IPM device handle");
		return -ENODEV;
	}

	ipm_register_callback(ipm_rx_handle, ipm_callback, NULL);

	vq[0] = virtqueue_allocate(VRING_SIZE);
	if (!vq[0]) {
		LOG_ERR("virtqueue_allocate failed to alloc vq[0]");
		return -ENOMEM;
	}

	vq[1] = virtqueue_allocate(VRING_SIZE);
	if (!vq[1]) {
		LOG_ERR("virtqueue_allocate failed to alloc vq[1]");
		return -ENOMEM;
	}

	rvrings[0].io = io;
	rvrings[0].info.vaddr = (void *)VRING_TX_ADDRESS;
	rvrings[0].info.num_descs = VRING_SIZE;
	rvrings[0].info.align = VRING_ALIGNMENT;
	rvrings[0].vq = vq[0];

	rvrings[1].io = io;
	rvrings[1].info.vaddr = (void *)VRING_RX_ADDRESS;
	rvrings[1].info.num_descs = VRING_SIZE;
	rvrings[1].info.align = VRING_ALIGNMENT;
	rvrings[1].vq = vq[1];

	vdev.role = RPMSG_REMOTE;
	vdev.vrings_num = VRING_COUNT;
	vdev.func = &dispatch;
	vdev.vrings_info = &rvrings[0];

	/* setup rvdev */
	err = rpmsg_init_vdev(&rvdev, &vdev, NULL, io, NULL);
	if (err) {
		LOG_ERR("rpmsg_init_vdev failed %d", err);
		return err;
	}

	rdev = rpmsg_virtio_get_rpmsg_device(&rvdev);

	err = rpmsg_create_ept(&ep, rdev, "dualcore", RPMSG_ADDR_ANY,
				  RPMSG_ADDR_ANY, endpoint_cb,
				  rpmsg_service_unbind);
	if (err) {
		LOG_ERR("rpmsg_create_ept failed %d", err);
		return err;
	}

	return err;
}

void main(void)
{
	int err;

	LOG_INF("Start");
	printk("start");

	if (event_manager_init()) {
		LOG_ERR("Event manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}

	/* initialize RPMSG */
	err = hci_rpmsg_init();
	if (err != 0) {
		return;
	}

	while (1) {
		k_sleep(K_SECONDS(1));
		//err = hci_rpmsg_send(buf);
		//if (err) {
		//	LOG_ERR("Failed to send (err %d)", err);
		//}
	}
}
