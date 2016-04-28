/* $NetBSD: $ */
/*-
  * Copyright (c) 2016 Manuel Bouyer
  * All rights reserved.
  * This software is distributed under the following condiions
  * compliant with the NetBSD foundation policy.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
  * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
  * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  * POSSIBILITY OF SUCH DAMAGE.
  */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/kernel.h>
#include <sys/fcntl.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/event.h>
#include <sys/gpio.h>
#include <sys/sysctl.h>

#include <dev/i2c/i2cvar.h>
#include <dev/gpio/gpiovar.h>
#include <dev/sysmon/sysmonvar.h>
#include <dev/sysmon/sysmon_taskq.h>

#if __arm__ /* XXX */
#include <arm/arm32/machdep.h>
static void picbmc_powerdown(void);
static struct picbmc_softc *powerdown_softc;
#endif

/* BMC registers */
#define PICBMC_REG_VERS		0x00
#define PICBMC_REG_STAT		0x01
#define PICBMC_REG_STAT_SW		(1 << 0)
#define PICBMC_REG_STAT_BLKON		(1 << 1)
#define PICBMC_REG_TEMPL	0x02
#define PICBMC_REG_TEMPH	0x03
#define PICBMC_REG_BATL		0x04
#define PICBMC_REG_BATH		0x05
#define PICBMC_REG_CMD		0x06
#define PICBMC_REG_CMD_PWROFF		(1 << 0)
#define PICBMC_REG_BLKPWM	0x07

struct picbmc_softc {
	device_t sc_dev;
	i2c_tag_t sc_tag;
	uint8_t	sc_address;
	struct	sysmon_envsys *sc_sme;
	envsys_data_t sc_sensor[2];
#define BMC_SENSOR_TEMP 0
#define BMC_SENSOR_BATT 1
	struct sysmon_pswitch sc_sw;
	bool	sc_sw_state;
	void	*sc_gpio;
	struct	gpio_pinmap sc_gpio_map;
	int	_gpio_map[1];
	int	sc_gpio_pin;
	kmutex_t sc_lwp_mtx;
	bool	sc_lwp_ev;

	struct sysctllog *sc_log;
};

static void	picbmc_attach(device_t, device_t, void *);
static int	picbmc_match(device_t, cfdata_t, void *);
static void	picbmc_config_gpio(device_t);

CFATTACH_DECL_NEW(picbmc, sizeof(struct picbmc_softc),
    picbmc_match, picbmc_attach, NULL, NULL);
extern struct cfdriver picbmc_cd;

static int picbmc_read_iic8(struct picbmc_softc *, uint8_t, uint8_t *);
static int picbmc_write_iic8(struct picbmc_softc *, uint8_t, uint8_t);
static int picbmc_read_iic16(struct picbmc_softc *, uint8_t, uint32_t *);
static void picbmc_refresh(struct sysmon_envsys *, envsys_data_t *);
static void picbmc_intr(void *);
static int picbmc_sysctl_blken(SYSCTLFN_PROTO);
static int picbmc_sysctl_blkpwm(SYSCTLFN_PROTO);

static int
picbmc_match(device_t parent, cfdata_t cf, void *arg)
{
	struct i2c_attach_args *ia = arg;

	if (ia->ia_name) {
		/* direct config - check name */
		if (strcmp(ia->ia_name, "picbmc") == 0)
			return 1;
	} else {
		/* indirect config - check typical address */
		if (ia->ia_addr == 0x69)
			return 1;
	}
	return 0;
}

static void
picbmc_attach(device_t parent, device_t self, void *arg)
{
	struct picbmc_softc *sc = device_private(self);
	struct i2c_attach_args *ia = arg;
	uint8_t vers;
	int error;
	const struct sysctlnode *node;

	aprint_naive(": PIC BMC\n");

	sc->sc_tag = ia->ia_tag;
	sc->sc_address = ia->ia_addr;
	sc->sc_dev = self;

#if __arm__ /* XXX */
	cpu_powerdown_address = picbmc_powerdown;
	powerdown_softc = sc;
#endif

	if (picbmc_read_iic8(sc,PICBMC_REG_VERS ,&vers) == 0) {
		aprint_error(": can't read version register\n");
	} else {
		aprint_normal(": PIC BMC version 0x%x\n", vers);
	}

	sc->sc_sme = sysmon_envsys_create();
	sc->sc_sme->sme_name = device_xname(self);
	sc->sc_sme->sme_cookie = sc;
	sc->sc_sme->sme_refresh = picbmc_refresh;

	sc->sc_sensor[BMC_SENSOR_TEMP].units =  ENVSYS_STEMP;
	sc->sc_sensor[BMC_SENSOR_TEMP].state = ENVSYS_SINVALID;
	sc->sc_sensor[BMC_SENSOR_TEMP].flags = 0;
	(void)strlcpy(sc->sc_sensor[BMC_SENSOR_TEMP].desc, "temperature",
	    sizeof(sc->sc_sensor[BMC_SENSOR_TEMP].desc));
	if ((error = sysmon_envsys_sensor_attach(sc->sc_sme,
	    &sc->sc_sensor[BMC_SENSOR_TEMP])) != 0) {
		aprint_error_dev(self, "unable to attach temp sensor (%d)\n",
		    error);
		goto bad;
	}
	sc->sc_sensor[BMC_SENSOR_BATT].units =  ENVSYS_SVOLTS_DC;
	sc->sc_sensor[BMC_SENSOR_BATT].state = ENVSYS_SINVALID;
	sc->sc_sensor[BMC_SENSOR_BATT].flags = ENVSYS_FHAS_ENTROPY;
	(void)strlcpy(sc->sc_sensor[BMC_SENSOR_BATT].desc, "battery",
	    sizeof(sc->sc_sensor[BMC_SENSOR_BATT].desc));
	if ((error = sysmon_envsys_sensor_attach(sc->sc_sme,
	    &sc->sc_sensor[BMC_SENSOR_BATT])) != 0) {
		aprint_error_dev(self, "unable to attach batt sensor(%d)\n",
		    error);
		goto bad;
	}

	error = sysmon_envsys_register(sc->sc_sme);
	if (error) {
		aprint_error_dev(self, 
		    "error %d registering with sysmon\n", error);
		goto bad;
	}
	config_interrupts(self, picbmc_config_gpio);

	sysctl_createv(&sc->sc_log, 0, NULL, &node, 0, 
	    CTLTYPE_NODE, device_xname(sc->sc_dev),
	    SYSCTL_DESCR("PIC BMC"),
	    NULL, 0, NULL, 0,
	    CTL_HW, CTL_CREATE, CTL_EOL);

	if (node == NULL) {
		aprint_error_dev(self, ": can't create sysctl node\n");
		return;
	}

	sysctl_createv(&sc->sc_log, 0, &node, NULL,
	    CTLFLAG_READONLY,
	    CTLTYPE_BOOL, "bklen",
	    SYSCTL_DESCR("backlight status"),
	    picbmc_sysctl_blken, 0, (void *)sc, 0,
	    CTL_CREATE, CTL_EOL);
	sysctl_createv(&sc->sc_log, 0, &node, NULL,
	    CTLFLAG_READWRITE,
	    CTLTYPE_INT, "bklpwm",
	    SYSCTL_DESCR("backlight percent"),
	    picbmc_sysctl_blkpwm, 0, (void *)sc, 0,
	    CTL_CREATE, CTL_EOL);
	return;
bad:
	sysmon_envsys_destroy(sc->sc_sme);
}

static void
picbmc_config_gpio(device_t self)
{
	struct picbmc_softc *sc = device_private(self);
	int caps;
	int error;

	sc->sc_gpio = gpio_find_device("gpio5"); /* XXXX */
	if (sc->sc_gpio == NULL) {
		aprint_error_dev(self, "can't locate gpio\n");
		return;
	}
	/* turn pin to intr */
	sc->sc_gpio_pin = /* 15 */ 0; /* XXX */
	aprint_normal_dev(self, ": using %s pin %d for interrupt\n",
	    gpio_get_name(sc->sc_gpio), sc->sc_gpio_pin);
	sc->sc_gpio_map.pm_map = sc->_gpio_map;
	if (gpio_pin_map(sc->sc_gpio, sc->sc_gpio_pin, 0x01,
	    &sc->sc_gpio_map)) {
		aprint_error_dev(self, ": can't map gpio pin\n");
		return;
	}
	KASSERT(sc->sc_gpio_map.pm_size == 1);
	KASSERT(sc->sc_gpio_map.pm_map[0] == sc->sc_gpio_pin);
	caps = gpio_pin_caps(sc->sc_gpio, &sc->sc_gpio_map, 0);
	if ((caps & (GPIO_PIN_EVENTS | GPIO_PIN_LEVEL)) !=
	    (GPIO_PIN_EVENTS | GPIO_PIN_LEVEL)) {
		aprint_error_dev(self, ": %s pin %d can't interrupt\n",
		    gpio_get_name(sc->sc_gpio), sc->sc_gpio_pin);
		gpio_pin_unmap(sc->sc_gpio, &sc->sc_gpio_map);
		return;
	}
	mutex_init(&sc->sc_lwp_mtx, MUTEX_DEFAULT, IPL_VM);
	sc->sc_lwp_ev = false;
	error = gpio_pin_ctl_intr(sc->sc_gpio, &sc->sc_gpio_map, 0,
	    GPIO_PIN_EVENTS | GPIO_PIN_LEVEL, IPL_VM, picbmc_intr, sc);
	if (error != 0) {
		aprint_error_dev(self, ": can't register GPIO interrupt (%d)\n",
		    error);
		gpio_pin_unmap(sc->sc_gpio, &sc->sc_gpio_map);
		return;
	}
	sc->sc_sw.smpsw_name = device_xname(sc->sc_dev);
	sc->sc_sw.smpsw_type = PSWITCH_TYPE_POWER;
	sysmon_pswitch_register(&sc->sc_sw);
	gpio_pin_irqen(sc->sc_gpio, &sc->sc_gpio_map, 0, true);
}

static void
picbmc_intr_task(void *arg)
{
	struct picbmc_softc *sc = arg;
	uint8_t val;
	bool new_state;

	mutex_enter(&sc->sc_lwp_mtx);
	if (!sc->sc_lwp_ev) {
		goto end;
	}

	sc->sc_lwp_ev = false;
	if (picbmc_read_iic8(sc, PICBMC_REG_STAT, &val) == 0) {
		printf("picbmc_intr_task: error reading BMC\n");
		goto end;
	}
	if (val & PICBMC_REG_STAT_SW) {
		new_state = 1;
	} else {
		new_state = 0;
	}
	if (sc->sc_sw_state != new_state) {
		sc->sc_sw_state = new_state;
		sysmon_pswitch_event(&sc->sc_sw,
		    new_state ?
		    PSWITCH_EVENT_PRESSED : PSWITCH_EVENT_RELEASED);
	}
end:
	gpio_pin_irqen(sc->sc_gpio, &sc->sc_gpio_map, 0, true);
	mutex_exit(&sc->sc_lwp_mtx);
}

static void
picbmc_intr(void *arg)
{
	struct picbmc_softc *sc = arg;

	mutex_enter(&sc->sc_lwp_mtx);
	if (!sc->sc_lwp_ev) {
		sc->sc_lwp_ev = true;
		sysmon_task_queue_sched(0, picbmc_intr_task, sc);
	}
	mutex_exit(&sc->sc_lwp_mtx);
}

static int
picbmc_sysctl_blken(SYSCTLFN_ARGS)
{
	struct sysctlnode node;
	struct picbmc_softc *sc;
	uint8_t reg;
	bool val;
	int error;

	node = *rnode;
	sc = node.sysctl_data;

	if (!picbmc_read_iic8(sc, PICBMC_REG_STAT, &reg)) {
		return EIO;
	}
	if (reg & PICBMC_REG_STAT_BLKON) {
		val = 1;
	} else {
		val = 0;
	}
	node.sysctl_data = &val;
	error = sysctl_lookup(SYSCTLFN_CALL(&node));
	return error;
}

static int
picbmc_sysctl_blkpwm(SYSCTLFN_ARGS)
{
	struct sysctlnode node;
	struct picbmc_softc *sc;
	uint8_t reg;
	int val;
	int error;

	node = *rnode;
	sc = node.sysctl_data;

	if (!picbmc_read_iic8(sc, PICBMC_REG_BLKPWM, &reg)) {
		return EIO;
	}
	if (reg == 0xff) {
		val = 100;
	} else {
		val = ((int)reg + 1) * 100 / 255;
	}
	node.sysctl_data = &val;
	error = sysctl_lookup(SYSCTLFN_CALL(&node));
	if (error || newp == NULL)
		return error;
	val = *(int *)node.sysctl_data;
	if (val < 10 || val > 100)
		return EINVAL;
	reg = val * 255 / 100;
	if (!picbmc_write_iic8(sc, PICBMC_REG_BLKPWM, reg)) {
		return EIO;
	}
	return 0;

}

static int
picbmc_read_iic8(struct picbmc_softc *sc, uint8_t reg, uint8_t *val)
{
	int error;

	if ((error = iic_acquire_bus(sc->sc_tag, I2C_F_POLL)) != 0) {
		aprint_error_dev(sc->sc_dev,
		    "%s: failed to acquire I2C bus: %d\n",
		    __func__, error);
		return 0;
	}

	error = iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP, sc->sc_address,
	     &reg, 1, val, 1, I2C_F_POLL);

	/* Done with I2C */
	iic_release_bus(sc->sc_tag, I2C_F_POLL);

	if (error != 0) {
		aprint_error_dev(sc->sc_dev,
		    "%s: failed to read register %d: %d\n",
		    __func__, reg, error);
		return 0;
	}
	return 1;
}

static int
picbmc_write_iic8(struct picbmc_softc *sc, uint8_t reg, uint8_t val)
{
	int error;
	uint8_t buf[2];

	buf[0] = reg;
	buf[1] = val;

	if ((error = iic_acquire_bus(sc->sc_tag, I2C_F_POLL)) != 0) {
		aprint_error_dev(sc->sc_dev,
		    "%s: failed to acquire I2C bus: %d\n",
		    __func__, error);
		return 0;
	}

	error = iic_exec(sc->sc_tag, I2C_OP_WRITE_WITH_STOP, sc->sc_address,
	     &buf, 2, NULL, 0, I2C_F_POLL);

	/* Done with I2C */
	iic_release_bus(sc->sc_tag, I2C_F_POLL);

	if (error != 0) {
		aprint_error_dev(sc->sc_dev,
		    "%s: failed to write register %d: %d\n",
		    __func__, reg, error);
		return 0;
	}
	return 1;
}

static int
picbmc_read_iic16(struct picbmc_softc *sc, uint8_t reg, uint32_t *val)
{
	int error;
	uint8_t buf[2];

	if ((error = iic_acquire_bus(sc->sc_tag, I2C_F_POLL)) != 0) {
		aprint_error_dev(sc->sc_dev,
		    "%s: failed to acquire I2C bus: %d\n",
		    __func__, error);
		return 0;
	}

	error = iic_exec(sc->sc_tag, I2C_OP_READ_WITH_STOP, sc->sc_address,
	     &reg, 1, buf, 2, I2C_F_POLL);

	/* Done with I2C */
	iic_release_bus(sc->sc_tag, I2C_F_POLL);

	if (error != 0) {
		aprint_error_dev(sc->sc_dev,
		    "%s: failed to read register %d: %d\n",
		    __func__, reg, error);
		return 0;
	}
	*val = ((buf[1] << 8) + buf[0]);
	return 1;
}

/* thermistor constants array */
struct picbmc_temp_val {
	uint32_t temp;
	uint32_t val;
} const picbmc_temps[] = {
	{
		.temp = 333000000,
		.val = 827, /* R = 2374 Ohm */
	},
	{
		.temp = 328000000,
		.val = 796, /* R = 2862 Ohm */
	},
	{
		.temp = 323000000,
		.val = 760, /* R = 3469 Ohm */
	},
	{
		.temp = 318000000,
		.val = 719, /* R = 4231 Ohm */
	},
	{
		.temp = 313000000,
		.val = 674, /* R = 5192 Ohm */
	},
	{
		.temp = 308000000,
		.val = 623, /* R = 6414 Ohm */
	},
	{
		.temp = 303000000,
		.val = 569, /* R = 7980 Ohm */
	},
	{
		.temp = 298000000,
		.val = 512, /* R = 10000 Ohm */
	},
	{
		.temp = 293000000,
		.val = 452, /* R = 12629 Ohm */
	},
	{
		.temp = 288000000,
		.val = 392, /* R = 16079 Ohm */
	},
	{
		.temp = 283000000,
		.val = 334, /* R = 20646 Ohm */
	},
	{
		.temp = 278000000,
		.val = 278, /* R = 26750 Ohm */
	},
	{
		.temp = 273000000,
		.val = 227, /* R = 34989 Ohm */
	},
	{
		.temp = 268000000,
		.val = 182, /* R = 46227 Ohm */
	},
	{
		.temp = 263000000,
		.val = 142, /* R = 61723 Ohm */
	},
	{
		.temp = 258000000,
		.val = 109, /* R = 83343 Ohm */
	},
	{
		.temp = 253000000,
		.val = 82, /* R = 113880 Ohm */
	},
};

static void
picbmc_refresh(struct sysmon_envsys *sme, envsys_data_t *edata)
{
	struct picbmc_softc *sc = sme->sme_cookie;
	uint32_t val;
	int i;

	switch(edata->sensor) {
	case BMC_SENSOR_TEMP:
		edata->state = ENVSYS_SINVALID;
		if (picbmc_read_iic16(sc, PICBMC_REG_TEMPL, &val) == 0) {
			return;
		}
		for (i = 1; i < __arraycount(picbmc_temps); i++) {
			if (val > picbmc_temps[i].val) {
				edata->state = ENVSYS_SVALID;
				break;
			}
		}
		if (edata->state == ENVSYS_SINVALID) {
			return;
		}
		KASSERT(i < __arraycount(picbmc_temps));
		KASSERT(i > 0);
		edata->value_cur = picbmc_temps[i - 1].temp -
		    ((picbmc_temps[i - 1].temp - picbmc_temps[i].temp)  /
		     (picbmc_temps[i - 1].val - picbmc_temps[i].val)  *
		     (picbmc_temps[i - 1].val - val));
		return;
	case BMC_SENSOR_BATT:
		if (picbmc_read_iic16(sc, PICBMC_REG_BATL, &val) == 0) {
			edata->state = ENVSYS_SINVALID;
			return;
		}
		edata->value_cur =
			/* 2048 * (110 / 10) * (val / 1024) * 1000 */
			val * 22000;
		edata->state = ENVSYS_SVALID;
		return;
	default:
		panic("picbmc_refresh");
	}
}

#if __arm__ /* XXX */
static void
picbmc_powerdown(void)
{
	uint8_t reg;
	struct picbmc_softc *sc = powerdown_softc;

	if (!picbmc_write_iic8(sc, PICBMC_REG_CMD, PICBMC_REG_CMD_PWROFF))
		return;

	while (1) {
		if (!picbmc_read_iic8(sc, PICBMC_REG_CMD, &reg))
			return;

		if   ((reg & PICBMC_REG_CMD_PWROFF) != PICBMC_REG_CMD_PWROFF)
			return;
	}
}
#endif
