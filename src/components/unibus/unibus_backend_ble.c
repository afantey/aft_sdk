/**
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-27     rgw          first version
 */

#include "unibus.h"
#include "unibus_board.h"
#include "sdk_board.h"

#define DBG_TAG "bus.ble"
#define DBG_LVL DBG_LOG
#include "sdk_log.h"

//////////////////////////BLE函数///////////////////////////////////////////////////

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>

#ifndef SDK_BLE_RX_FIFO_LEN
#define SDK_BLE_RX_FIFO_LEN    512
#endif

typedef struct ble_rx_fifo ble_rx_fifo_t;

struct ble_rx_fifo
{
    uint8_t buffer[SDK_BLE_RX_FIFO_LEN];
    uint16_t put_index, get_index;
    bool is_full;
};

static ble_rx_fifo_t ble_rx_fifo;

static void ble_getc(uint8_t ch)
{
    ble_rx_fifo_t *rx_fifo;

    rx_fifo = &ble_rx_fifo;

    rx_fifo->buffer[rx_fifo->put_index] = ch;
    rx_fifo->put_index += 1;
    if (rx_fifo->put_index >= SDK_BLE_RX_FIFO_LEN)
        rx_fifo->put_index = 0;

    /* if the next position is read index, discard this 'read char' */
    if (rx_fifo->put_index == rx_fifo->get_index)
    {
        rx_fifo->get_index += 1;
        rx_fifo->is_full = true;
        if (rx_fifo->get_index >= SDK_BLE_RX_FIFO_LEN)
            rx_fifo->get_index = 0;
    }
}

static int32_t ble_read(uint8_t *data, int32_t len)
{
    int size;
    ble_rx_fifo_t *rx_fifo;

    size = len;

    rx_fifo = &ble_rx_fifo;

    while (len)
    {
        int ch;
        
        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == false))
        {
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= SDK_BLE_RX_FIFO_LEN)
            rx_fifo->get_index = 0;

        if (rx_fifo->is_full == true)
        {
            rx_fifo->is_full = false;
        }

        *data = ch & 0xff;
        data++;
        len--;
    }

    return size - len;
}

//(1) 定义 UUID
/* TRx Service Variables */
#define BT_UUID_TRX_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static struct bt_uuid_128 trx_svc_uuid = BT_UUID_INIT_128(
	BT_UUID_TRX_SERVICE_VAL);

static struct bt_uuid_128 trx_c2s_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

static struct bt_uuid_128 trx_s2c_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2));


//(2) 定义 GATT 服务
#define TRX_MAX_LEN 20

static uint8_t trx_c2s_value[TRX_MAX_LEN + 1] = {
	'T', 'R', 'X', '_', 'C', '2', 'S'
};
static uint8_t trx_s2c_value[TRX_MAX_LEN + 1] = {
	'T', 'R', 'X', '_', 'S', '2', 'C'
};

static ssize_t write_trx_c2s(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > TRX_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

    for(int i = 0; i < len; i++)
    {
        ble_getc(value[i]);
    }

	LOG_D("write (%d): ", len);
    LOG_HEX(64, value, len);

	return len;
}

static ssize_t read_trx_s2c(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 strlen(value));
}

static uint8_t simulate;

static void trx_s2c_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	simulate = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/* TRx Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(trx_svc,
	BT_GATT_PRIMARY_SERVICE(&trx_svc_uuid),
	BT_GATT_CHARACTERISTIC(&trx_c2s_uuid.uuid,
			       BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE,
			       NULL, write_trx_c2s, trx_c2s_value),
	BT_GATT_CHARACTERISTIC(&trx_s2c_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_trx_s2c, NULL, trx_s2c_value),
	BT_GATT_CCC(trx_s2c_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);


//(3) 定义接口
static struct bt_gatt_attr *trx_ntf_attr;

static void trx_init(void)
{
	trx_ntf_attr = bt_gatt_find_by_uuid(trx_svc.attrs, trx_svc.attr_count, &trx_s2c_uuid.uuid);
}

static int trx_notify(const uint8_t *pbuff, int len)
{
    int block_num = len / 20;
    int bytes_remain = len % 20;

    for(int i = 0; i < block_num; i++)
    {
        if (simulate)
        {
            bt_gatt_notify(NULL, trx_ntf_attr, pbuff + i * 20, 20);
        }
    }

    if(bytes_remain > 0)
    {
        if (simulate)
        {
            bt_gatt_notify(NULL, trx_ntf_attr, pbuff + block_num * 20, bytes_remain);
        }
    }
    return len;
}

//(4) 修改广播数据
//添加了 TRx Service UUID，以便在广播数据中发现它。
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_TRX_SERVICE_VAL),
};




/** current test: 1S adv_interval 1S conn_interval*/
#define TEST_LOW_POWER_CURRENT_1S      0

/** current test: 100ms adv_interval 100ms conn_interval*/
#define TEST_LOW_POWER_CURRENT_100MS      0

#if TEST_LOW_POWER_CURRENT_1S
#define BT_LE_ADV_CONN_TEST BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
					    BT_LE_ADV_OPT_USE_NAME, \
					    BT_GAP_ADV_SLOW_INT_MIN, \
					    BT_GAP_ADV_SLOW_INT_MIN, NULL)
#endif


#if TEST_LOW_POWER_CURRENT_100MS
#define BT_LE_ADV_CONN_TEST BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
					    BT_LE_ADV_OPT_USE_NAME, \
					    BT_GAP_ADV_FAST_INT_MIN_2, \
					    BT_GAP_ADV_FAST_INT_MIN_2, NULL)
#endif

#if ((TEST_LOW_POWER_CURRENT_1S + TEST_LOW_POWER_CURRENT_100MS) > 1)
#error "Only one test instance can be enabled at a time. "
#endif


// static const struct bt_data ad[] = {
// 	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
// 	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
// 		      BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
// 		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
// 		      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL))
// };

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_E("Connection failed (err 0x%02x)\n", err);
	} else {
		LOG_I("Connected\n");

		#if TEST_LOW_POWER_CURRENT_1S
		const struct bt_le_conn_param *param;

		param = BT_LE_CONN_PARAM(800, 800, 0, 500);
		bt_conn_le_param_update(conn, param);
		#endif

		#if TEST_LOW_POWER_CURRENT_100MS
		const struct bt_le_conn_param *param;

		param = BT_LE_CONN_PARAM(80, 80, 0, 500);
		bt_conn_le_param_update(conn, param);
		#endif
	}


}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_I("Disconnected (reason 0x%02x)\n", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	int err;

	LOG_I("Bluetooth initialized\n");

	#if (TEST_LOW_POWER_CURRENT_1S || TEST_LOW_POWER_CURRENT_100MS)
	err = bt_le_adv_start(BT_LE_ADV_CONN_TEST, ad, ARRAY_SIZE(ad), NULL, 0);
	#else
	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	#endif

	if (err) {
		LOG_E("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_I("Advertising successfully started\n");
}


//////////////////////////需要不同平台对接的函数///////////////////////////////////////////////////
// int unibus_ble_read(void *pctx, uint8_t *rbuff, int len, int wait);
// int unibus_ble_write(void *pctx, uint8_t *pbuff, int len);
///////////////////////////////////////////////////////////////////////////////////////
static int unibus_ble_write(void *pctx, const uint8_t *pbuff, int len)
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;

    return trx_notify(pbuff, len);
}

struct unibus_ble_read_pt_data
{
    uint32_t sum;
    int timeout_ms;
    int remaining_len;
    int step; //0 验证阶段  1 读取阶段
    uni_data_16_t uni_body_len;
};

static struct unibus_ble_read_pt_data ble_pt_data = {
    .sum = 0,
    .timeout_ms = 0,
    .remaining_len = 0,
    .step = 0,
    .uni_body_len = {0},
};
#define LOOP_DELAY 5
static PT_THREAD(unibus_ble_read(void *pctx, uint8_t *rbuff, int len, uint32_t *psum, int wait))
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;
    uint32_t rl = 0;
    struct unibus_ble_read_pt_data *pt_data = (struct unibus_ble_read_pt_data *)ctx->backend->pt_data; 

    PT_BEGIN(&ctx->pt_backend_read);
    
    memset(pt_data, 0, sizeof(struct unibus_ble_read_pt_data));
    pt_data->timeout_ms = ctx->response_timeout;
    
    do {
        swtimer_set(&ctx->swtimer_backend_read, LOOP_DELAY);
        if (len - pt_data->sum == 0)
            break;
        if (pt_data->step == 0) {
            rl = ble_read(rbuff + pt_data->sum, 1);
            if (rl == 1) {
                switch (pt_data->sum) {
                case 0:
                    if (rbuff[0] == 0xAB)
                        pt_data->sum++;
                    break;
                case 1:
                    if (rbuff[1] == 0xBA)
                        pt_data->sum++;
                    else if (rbuff[1] == 0xAB)
                        rbuff[0] = 0xAB;
                    else
                        pt_data->sum--;
                    break;
                case 2:
                    pt_data->uni_body_len.data_8[0] = rbuff[2];
                    pt_data->sum++;
                    break;
                  case 3:
                    pt_data->uni_body_len.data_8[1] = rbuff[3];
                    pt_data->remaining_len = pt_data->uni_body_len.data_16 + 7;
                    pt_data->sum++;
                    pt_data->step = 1;
                    break;
                default:
                    break;
                }
            } else if (wait == 0 && pt_data->sum == 0) //不等模式
            {
                break;
            }
        }
        if (pt_data->step == 1) {
            rl = ble_read(rbuff + pt_data->sum, pt_data->remaining_len);
            pt_data->sum += rl;
            pt_data->remaining_len -= rl;
            if (pt_data->sum == pt_data->uni_body_len.data_16 + 7 + 4) {
                break; //OK
            }
        }
        if (pt_data->timeout_ms == 0) {
            pt_data->sum = 0;
            break;
        }
        if (rl > 0) {
            continue;
        } else {
            PT_YIELD_UNTIL(&ctx->pt_backend_read, swtimer_expired(&ctx->swtimer_backend_read));
            pt_data->timeout_ms -= LOOP_DELAY;
            LOG_D("timeout %d\n", pt_data->timeout_ms);
        }
    } while (pt_data->timeout_ms > 0);

    *psum = pt_data->sum;
    
    PT_END(&ctx->pt_backend_read);
}

/**
 * @brief 计算校验码
 * 
 * @param data 除AB，BA的内容
 * @param len 
 * @return uint16_t 
 */
static uint16_t checksumLRC(const uint8_t *data, int len)
{
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return -sum;
}

/**
 * @brief 制作串口发送帧
 * 
 * @param pctx 
 * @param p_msg_meta 
 * @param msg 
 * @param p_msg_len 
 * @return int 
 */
static int unibus_ble_make_msg(void *pctx, msg_meta_t *p_msg_meta, uint8_t *msg, int *p_msg_len)
{
    int offset = 0;
    uni_data_16_t checksum = {0};

    msg[offset++] = 0xAB;
    msg[offset++] = 0xBA;
    msg[offset++] = p_msg_meta->uni_body_len.data_8[0];
    msg[offset++] = p_msg_meta->uni_body_len.data_8[1];
    msg[offset++] = p_msg_meta->src_id;
    msg[offset++] = p_msg_meta->dest_id;
    msg[offset++] = p_msg_meta->cmd_func;
    msg[offset++] = p_msg_meta->uni_index.data_8[0];
    msg[offset++] = p_msg_meta->uni_index.data_8[1];
    for (int i = 0; i < p_msg_meta->uni_body_len.data_16; i++) {
        msg[offset++] = p_msg_meta->body[i];
    }
    checksum.data_16 = checksumLRC(msg + 2, offset - 2);
    msg[offset++] = checksum.data_8[0];
    msg[offset++] = checksum.data_8[1];
    *p_msg_len = offset;

    return UNI_ERR_NO;
}

/**
 * @brief 校验串口接收帧
 * 
 * @param pctx 
 * @param msg 
 * @param msg_len 
 * @param p_msg_meta 返回接收到的协议结构
 * @return int 
 */
static int unibus_ble_check_integrity(void *pctx, uint8_t *msg, int msg_len, msg_meta_t *p_msg_meta)
{
    unibus_t *ctx = (unibus_t *)pctx;
    uint16_t body_len = 0;
    uni_data_16_t checksum = {0};

    if (msg[0] != 0xAB || msg[1] != 0xBA)
    {
        return UNI_ERR_PROTO_BROKEN;
    }
    body_len = msg[2] + (msg[3] << 8);
    if (msg_len < (body_len + ctx->backend->header_length + ctx->backend->checksum_length) ||
        msg_len > (ctx->backend->max_data_length + ctx->backend->header_length + ctx->backend->checksum_length))
    {
        return UNI_ERR_PROTO_BROKEN;
    }
    checksum.data_16 = checksumLRC(msg + 2, body_len + 7); //7为协议中除帧头其余固定参数长度
    if (checksum.data_8[0] != msg[msg_len - 2] || checksum.data_8[1] != msg[msg_len - 1])
    {
        LOG_D("check orig[0]%02x [1]%02x, calc [0]%02x [1]%02x\n", msg[msg_len - 2], msg[msg_len - 1], checksum.data_8[0], checksum.data_8[1]);
        return UNI_ERR_PROTO_CHECK;
    }
    p_msg_meta->src_id = msg[4]; //原id
    p_msg_meta->dest_id = msg[5]; //目标id //接收时，目标id为接收端发送时候的原id
    p_msg_meta->cmd_func = msg[6]; //功能码
    p_msg_meta->uni_index.data_16 = msg[7] + (msg[8] << 8);
    for(int i = 0; i < body_len; i++)
    {
        p_msg_meta->body[i] = msg[ctx->backend->header_length + i];
    }
    p_msg_meta->uni_body_len.data_16 = body_len;
    return UNI_ERR_NO;
}

#if 0
static int _unibus_ble_flush(unibus_t *ctx)
{
    uint8_t buff[128];
    uint32_t l = 0;
    uint32_t sum = 0;
    while ((l = sdk_uart_read(ctx->port, buff, 128)) > 0)
    {
        sum += l;
    }
    return sum;
}
#endif

static int unibus_ble_connect(void *pctx)
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;

    // sdk_uart_open((sdk_uart_t *)ctx->port, 115200, 8, 'N', 1);
    // sdk_uart_control((sdk_uart_t *)ctx->port, SDK_CONTROL_UART_ENABLE_INT, NULL);

	int err;

	err = bt_enable(NULL);

	trx_init();

	if (err) {
		LOG_E("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	bt_ready();

    return UNI_ERR_NO;
}

static void unibus_ble_close(void *pctx)
{
    unibus_t *ctx;
    ctx = (unibus_t *)pctx;

    // sdk_uart_close((sdk_uart_t *)ctx->port);
}

const unibus_backend_t backend_ble = {
    .header_length    =  _UNIBUS_BLE_HEADER_LENGTH  ,
    .checksum_length  =  _UNIBUS_BLE_CHECKSUM_LENGTH,
    .max_data_length  =  _UNIBUS_BLE_MAX_DATA_LENGTH,
    .pt_data          = &ble_pt_data                ,
    .read             =  unibus_ble_read            ,
    .write            =  unibus_ble_write           ,
    .make_send_msg    =  unibus_ble_make_msg        ,
    .check_integrity  =  unibus_ble_check_integrity ,
    .receive          =  NULL                       ,
    .prepare_response =  NULL                       ,
    .connect          =  unibus_ble_connect         ,
    .close            =  unibus_ble_close           ,
};
