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
static d_poll_t			ti_pruss_irq_poll;

static device_probe_t		ti_pruss_probe;
static device_attach_t		ti_pruss_attach;
static device_detach_t		ti_pruss_detach;
static void			ti_pruss_intr(void *);
static d_open_t			ti_pruss_open;
static d_mmap_t			ti_pruss_mmap;
static void 			ti_pruss_irq_kqread_detach(struct knote *);
static int 			ti_pruss_irq_kqevent(struct knote *, long);
static d_kqfilter_t		ti_pruss_irq_kqfilter;
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

static struct cdevsw ti_pruss_cdevirq = {
	.d_version =	D_VERSION,
	.d_name =	"ti_pruss_irq",
	.d_open =	ti_pruss_irq_open,
	.d_read =	ti_pruss_irq_read,
	.d_poll =	ti_pruss_irq_poll,
	.d_kqfilter =	ti_pruss_irq_kqfilter,
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
ti_pruss_irq_poll(struct cdev *dev, int events, struct thread *td)
{
	struct ctl* irqs;
	struct ti_pruss_irqsc *sc;
	sc = dev->si_drv1;

	devfs_get_cdevpriv((void**)&irqs);

	if (events & (POLLIN | POLLRDNORM)) {
		if (sc->tstamps.ctl.cnt != irqs->cnt)
			return events & (POLLIN | POLLRDNORM);
		else
			selrecord(td, &sc->sc_selinfo);
	}
	return 0;
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

static struct filterops ti_pruss_kq_read = {
	.f_isfd = 1,
	.f_detach = ti_pruss_irq_kqread_detach,
	.f_event = ti_pruss_irq_kqevent,
};

static void
ti_pruss_irq_kqread_detach(struct knote *kn)
{
	struct ti_pruss_irqsc *sc = kn->kn_hook;

	knlist_remove(&sc->sc_selinfo.si_note, kn, 0);
}

static int
ti_pruss_irq_kqevent(struct knote *kn, long hint)
{
    struct ti_pruss_irqsc* irq_sc;
    int notify;

    irq_sc = kn->kn_hook;

    if (hint > 0)
        kn->kn_data = hint - 2;

    if (hint > 0 || irq_sc->last > 0)
        notify = 1;
    else
        notify = 0;

    irq_sc->last = hint;

    return (notify);
}

static int
ti_pruss_irq_kqfilter(struct cdev *cdev, struct knote *kn)
{
	struct ti_pruss_irqsc *sc = cdev->si_drv1;

	switch (kn->kn_filter) {
	case EVFILT_READ:
		kn->kn_hook = sc;
		kn->kn_fop = &ti_pruss_kq_read;
		knlist_add(&sc->sc_selinfo.si_note, kn, 0);
		break;
	default:
		return (EINVAL);
	}

	return (0);
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
  .poll_h = ti_pruss_irq_poll,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};