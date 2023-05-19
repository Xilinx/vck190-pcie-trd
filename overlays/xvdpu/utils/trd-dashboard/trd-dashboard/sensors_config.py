# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0

# PySensors does not honor lmsensors configuration files under /etc/sensors.d,
# therefore replicate a simplified configuration dictionary here focusing on
# labels and compute statements only for now.
sensor_pfm23 = {
    'ina226-i2c-5-4d' : {
        'power1' : {
            'label' : 'mgtyvccaux'
        }
    },
    'ina226-i2c-5-4b' : {
        'power1' : {
            'label' : 'mgtyavcc'
        }
    },
    'ina226-i2c-5-45' : {
        'power1' : {
            'label' : 'vcc mio'
        }
    },
    'ina226-i2c-5-49' : {
        'power1' :{
            'label' : 'vcc1v1 lp4'
        }
    },
    'ina226-i2c-5-40' : {
        'power1' : {
            'label' : 'vccaux'
        }
    },
    'ina226-i2c-5-47' : {
        'power1' : {
            'label' : 'vcc3v3'
        }
    },
    'ina226-i2c-3-44' : {
        'power1' : {
            'label' : 'vcc pslp'
        }
    },
    'ina226-i2c-3-42' : {
        'power1' : {
            'label' : 'vccint pmc'
        }
    },
    'ina226-i2c-3-40' : {
        'power1' : {
            'label' : 'vccint',
            'compute' : '*6'
        }
    },
    'ina226-i2c-5-4c' : {
        'power1' : {
            'label' : 'mgtyavtt'
        }
    },
    'ina226-i2c-5-46' : {
        'power1' : {
            'label' : 'vcc1v8'
        }
    },
    'ina226-i2c-5-4a' : {
        'power1' : {
            'label' : 'vadj fmc'
        }
    },
    'ina226-i2c-5-41' : {
        'power1' : {
            'label' : 'vccaux pmc'
        }
    },
    'ina226-i2c-5-48' : {
        'power1' : {
            'label' : 'vcc1v2 ddr4'
        }
    },
    'ina226-i2c-3-45' : {
        'power1' : {
            'label' : 'vcc psfp'
        }
    },
    'ina226-i2c-3-43' : {
        'power1' : {
            'label' : 'vcc ram io'
        }
    },
    'ina226-i2c-3-41' : {
        'power1' : {
            'label' : 'vcc soc'
        }
    },
    'iio_hwmon-isa-0000' : {
        'temp1': {
            'label' : 'temp',
            'compute' : '*1000/128'
        },
        'temp2': {
            'label' : 'temp min',
            'compute' : '*1000/128'
        },
        'temp3': {
            'label' : 'temp max max',
            'compute' : '*1000/128'
        },
        'temp4': {
            'label' : 'temp min min',
            'compute' : '*1000/128'
        }
    }
}

sensor_pfm1 = {
    'ina226-i2c-6-4d' : {
        'power1' : {
            'label' : 'mgtyvccaux'
        }
    },
    'ina226-i2c-6-4b' : {
        'power1' : {
            'label' : 'mgtyavcc'
        }
    },
    'ina226-i2c-6-45' : {
        'power1' : {
            'label' : 'vcc mio'
        }
    },
    'ina226-i2c-6-49' : {
        'power1' :{
            'label' : 'vcc1v1 lp4'
        }
    },
    'ina226-i2c-6-40' : {
        'power1' : {
            'label' : 'vccaux'
        }
    },
    'ina226-i2c-6-47' : {
        'power1' : {
            'label' : 'vcc3v3'
        }
    },
    'ina226-i2c-4-44' : {
        'power1' : {
            'label' : 'vcc pslp'
        }
    },
    'ina226-i2c-4-42' : {
        'power1' : {
            'label' : 'vccint pmc'
        }
    },
    'ina226-i2c-4-40' : {
        'power1' : {
            'label' : 'vccint',
            'compute' : '*6'
        }
    },
    'ina226-i2c-6-4c' : {
        'power1' : {
            'label' : 'mgtyavtt'
        }
    },
    'ina226-i2c-6-46' : {
        'power1' : {
            'label' : 'vcc1v8'
        }
    },
    'ina226-i2c-6-4a' : {
        'power1' : {
            'label' : 'vadj fmc'
        }
    },
    'ina226-i2c-6-41' : {
        'power1' : {
            'label' : 'vccaux pmc'
        }
    },
    'ina226-i2c-6-48' : {
        'power1' : {
            'label' : 'vcc1v2 ddr4'
        }
    },
    'ina226-i2c-4-45' : {
        'power1' : {
            'label' : 'vcc psfp'
        }
    },
    'ina226-i2c-4-43' : {
        'power1' : {
            'label' : 'vcc ram io'
        }
    },
    'ina226-i2c-4-41' : {
        'power1' : {
            'label' : 'vcc soc'
        }
    },
    'iio_hwmon-isa-0000' : {
        'temp1': {
            'label' : 'temp',
            'compute' : '*1000/128'
        },
        'temp2': {
            'label' : 'temp min',
            'compute' : '*1000/128'
        },
        'temp3': {
            'label' : 'temp max max',
            'compute' : '*1000/128'
        },
        'temp4': {
            'label' : 'temp min min',
            'compute' : '*1000/128'
        }
    }
}
