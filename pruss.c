//TODO: License

#include <machine/rtems-bsd-kernel-space.h>

#include <sys/cdefs.h>

#include <sys/poll.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/fcntl.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/event.h>
#include <sys/selinfo.h>
//#include <machine/frame.h> can't be found.
#include <machine/intr.h>
#include <machine/atomic.h>

#include <machine/bus.h>
#include <machine/cpu.h>

#include <rtems/imfs.h>

#include "pruss.h"

static d_open_t			ti_pruss_irq_open;
static d_read_t			ti_pruss_irq_read;

static void			ti_pruss_privdtor(void *data);

#define	TI_PRUSS_PRU_IRQS 2
#define	TI_PRUSS_HOST_IRQS 8
#define	TI_PRUSS_IRQS (TI_PRUSS_HOST_IRQS+TI_PRUSS_PRU_IRQS)
#define	TI_PRUSS_EVENTS 64
#define	NOT_SET_STR "NONE"
#define	TI_TS_ARRAY 16

struct ctl
{
	size_t cnt;
	size_t idx;
};

struct ts_ring_buf
{
	struct ctl ctl;
	uint64_t ts[TI_TS_ARRAY];
};

struct ti_pruss_irqsc
{
	struct mtx		sc_mtx;
	struct cdev		*sc_pdev;
	struct selinfo		sc_selinfo;
	int8_t			channel;
	int8_t			last;
	int8_t			event;
	bool			enable;
	struct ts_ring_buf	tstamps;
};



static int
ti_pruss_irq_open(struct cdev *dev, int oflags, int devtype, struct thread *td)
{
	struct ctl* irqs;
	struct ti_pruss_irqsc *sc;
	sc = dev->si_drv1;

	irqs = malloc(sizeof(struct ctl), M_DEVBUF, M_WAITOK);
	if (!irqs)
	    return (ENOMEM);

	irqs->cnt = sc->tstamps.ctl.cnt;
	irqs->idx = sc->tstamps.ctl.idx;

	return devfs_set_cdevpriv(irqs, ti_pruss_privdtor);
}

static int
ti_pruss_irq_read(struct cdev *cdev, struct uio *uio, int ioflag)
{
	const size_t ts_len = sizeof(uint64_t);
	struct ti_pruss_irqsc* irq;
	struct ctl* priv;
	int error = 0;
	size_t idx;
	ssize_t level;

	irq = cdev->si_drv1;

	if (uio->uio_resid < ts_len)
		return (EINVAL);

	error = devfs_get_cdevpriv((void**)&priv);
	if (error)
	    return (error);

	mtx_lock(&irq->sc_mtx);

	if (irq->tstamps.ctl.cnt - priv->cnt > TI_TS_ARRAY)
	{
		priv->cnt = irq->tstamps.ctl.cnt;
		priv->idx = irq->tstamps.ctl.idx;
		mtx_unlock(&irq->sc_mtx);
		return (ENXIO);
	}

	do {
		idx = priv->idx;
		level = irq->tstamps.ctl.idx - idx;
		if (level < 0)
			level += TI_TS_ARRAY;

		if (level == 0) {
			if (ioflag & O_NONBLOCK) {
				mtx_unlock(&irq->sc_mtx);
				return (EWOULDBLOCK);
			}

			error = msleep(irq, &irq->sc_mtx, PCATCH | PDROP,
				"pruirq", 0);
			if (error)
				return error;

			mtx_lock(&irq->sc_mtx);
		}
	}while(level == 0);

	mtx_unlock(&irq->sc_mtx);

	error = uiomove(&irq->tstamps.ts[idx], ts_len, uio);

	if (++idx == TI_TS_ARRAY)
		idx = 0;
	priv->idx = idx;

	atomic_add_32(&priv->cnt, 1);

	return (error);
}


static const rtems_filesystem_file_handlers_r pruss_irq_handler = {
  .open_h = ti_pruss_irq_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = ti_pruss_irq_read,
  .write_h = rtems_filesystem_default_write,
  .ioctl_h = rtems_filesystem_default_ioctl,
  .lseek_h = rtems_filesystem_default_lseek,
  .fstat_h = IMFS_stat,
  .ftruncate_h = rtems_filesystem_default_ftruncate,
  .fsync_h = rtems_filesystem_default_fsync_or_fdatasync,
  .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync,
  .fcntl_h = rtems_filesystem_default_fcntl,
  .kqfilter_h = rtems_filesystem_default_kqfilter,
  .mmap_h = rtems_filesystem_default_mmap,
  .poll_h = rtems_filesystem_default_poll,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};