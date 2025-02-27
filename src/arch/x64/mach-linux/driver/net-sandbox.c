/*
 * driver/net-sandbox.c
 *
 * Copyright(c) 2007-2022 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <net/net.h>
#include <sandbox.h>

struct net_sandbox_pdata_t {
	char * iface;
};

static struct socket_listen_t * net_sandbox_listen(struct net_t * net, const char * type, int port)
{
	void * lctx;

	lctx = sandbox_socket_listen(type, port);
	if(lctx)
		return socket_listen_alloc(net, lctx);
	return NULL;
}

static struct socket_connect_t * net_sandbox_accept(struct socket_listen_t * l)
{
	void * cctx;

	cctx = sandbox_socket_accept(l->priv);
	if(cctx)
		return socket_connect_alloc(l->net, cctx);
	return NULL;
}

static struct socket_connect_t * net_sandbox_connect(struct net_t * net, const char * type, const char * host, int port)
{
	void * cctx;

	cctx = sandbox_socket_connect(type, host, port);
	if(cctx)
		return socket_connect_alloc(net, cctx);
	return NULL;
}

static int net_sandbox_read(struct socket_connect_t * c, void * buf, int count)
{
	return sandbox_socket_read(c->priv, buf, count);
}

static int net_sandbox_write(struct socket_connect_t * c, void * buf, int count)
{
	return sandbox_socket_write(c->priv, buf, count);
}

static int net_sandbox_status(struct socket_connect_t * c)
{
	return sandbox_socket_status(c->priv);
}

static void net_sandbox_close(struct socket_connect_t * c)
{
	sandbox_socket_close(c->priv);
	socket_connect_free(c);
}

static void net_sandbox_delete(struct socket_listen_t * l)
{
	sandbox_socket_delete(l->priv);
	socket_listen_free(l);
}

static int net_sandbox_ioctl(struct net_t * net, const char * cmd, void * arg)
{
	struct net_sandbox_pdata_t * pdat = (struct net_sandbox_pdata_t *)net->priv;

	switch(shash(cmd))
	{
	case 0xccaf0ac8: /* "net-get-type" */
		if(arg)
		{
			strcpy((char *)arg, "sandbox");
			return 0;
		}
		break;

	case 0x74c961bf: /* "net-get-ip" */
		if(arg)
		{
			if(sandbox_socket_get_ip(pdat->iface, arg))
				return 0;
		}
		break;

	case 0x0df5a917: /* "net-get-mac" */
		if(arg)
		{
			if(sandbox_socket_get_mac(pdat->iface, arg))
				return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static struct device_t * net_sandbox_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct net_sandbox_pdata_t * pdat;
	struct net_t * net;
	struct device_t * dev;

	pdat = malloc(sizeof(struct net_sandbox_pdata_t));
	if(!pdat)
		return NULL;

	net = malloc(sizeof(struct net_t));
	if(!net)
		return NULL;

	pdat->iface = strdup(dt_read_string(n, "interface", "eth0"));

	net->name = alloc_device_name(dt_read_name(n), -1);
	net->listen = net_sandbox_listen;
	net->accept = net_sandbox_accept;
	net->connect = net_sandbox_connect;
	net->read = net_sandbox_read;
	net->write = net_sandbox_write;
	net->status = net_sandbox_status;
	net->close = net_sandbox_close;
	net->delete = net_sandbox_delete;
	net->ioctl = net_sandbox_ioctl;
	net->priv = pdat;

	if(!(dev = register_net(net, drv)))
	{
		free_device_name(net->name);
		free(net->priv);
		free(net);
		return NULL;
	}
	return dev;
}

static void net_sandbox_remove(struct device_t * dev)
{
	struct net_t * net = (struct net_t *)dev->priv;

	if(net)
	{
		unregister_net(net);
		free_device_name(net->name);
		free(net->priv);
		free(net);
	}
}

static void net_sandbox_suspend(struct device_t * dev)
{
}

static void net_sandbox_resume(struct device_t * dev)
{
}

static struct driver_t net_sandbox = {
	.name		= "net-sandbox",
	.probe		= net_sandbox_probe,
	.remove		= net_sandbox_remove,
	.suspend	= net_sandbox_suspend,
	.resume		= net_sandbox_resume,
};

static __init void net_sandbox_driver_init(void)
{
	register_driver(&net_sandbox);
}

static __exit void net_sandbox_driver_exit(void)
{
	unregister_driver(&net_sandbox);
}

driver_initcall(net_sandbox_driver_init);
driver_exitcall(net_sandbox_driver_exit);
