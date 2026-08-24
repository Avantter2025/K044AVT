package com.avanttec.fingerprint;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Estrutura JNA mapeando k044_fp_nfpage_t (página de informações do módulo).
 * A ordem e os tipos dos campos espelham exatamente o struct em display_driver.h.
 */
public class NFPage extends Structure {
    public short registros;
    public short temp_size;
    public short database_size;
    public short score_level;
    public byte[] device_address = new byte[4];
    public short cfg_pkt_size;
    public short cfg_baud_rate;
    public short anti_fake;
    public short fp_sensor;
    public short secur_level;
    public short enroll_logic;
    public short image_format;
    public short delay_time;
    public byte[] product_sn = new byte[17];
    public byte[] software_version = new byte[17];
    public byte[] manufacturer = new byte[17];
    public byte[] sensor_name = new byte[17];
    public byte[] password = new byte[4];
    public byte[] jtag_flag = new byte[4];

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(
                "registros", "temp_size", "database_size", "score_level",
                "device_address", "cfg_pkt_size", "cfg_baud_rate", "anti_fake",
                "fp_sensor", "secur_level", "enroll_logic", "image_format",
                "delay_time", "product_sn", "software_version", "manufacturer",
                "sensor_name", "password", "jtag_flag");
    }

    /** Converte um campo char[] (terminado em NUL) em String legível. */
    static String cstr(byte[] raw) {
        int n = 0;
        while (n < raw.length && raw[n] != 0) n++;
        return new String(raw, 0, n).trim();
    }
}
