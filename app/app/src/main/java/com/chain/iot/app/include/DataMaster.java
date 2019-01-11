package com.chain.iot.app.include;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.le.BluetoothLeScanner;

import java.util.UUID;

public class DataMaster extends Data
{
    private BluetoothDevice device;//bluetooth device(remote)
    private int rssi;//Received Signal Strength Indicator(between master and slave)
    private boolean condition;//device condition(remote)
    private BluetoothGatt gatt;//bluetooth gatt
    private BluetoothGattService service;//bluetooth gatt service(remote)
    private BluetoothGattCharacteristic characteristic;//bluetooth gatt characteristic(remote)
    //
    public DataMaster()
    {
        super();
        //
        set_status(Include.STATUS_DISCONNECT);
    }
    public void set_device(BluetoothDevice device)
    {
        this.device=device;
    }
    public BluetoothDevice get_device()
    {
        return device;
    }
    public void set_rssi(int rssi)
    {
        this.rssi=rssi;
    }
    public int get_rssi()
    {
        return rssi;
    }
    public void set_condition(boolean condition)
    {
        this.condition=condition;
    }
    public boolean get_condition()
    {
        return condition;
    }
    public void set_gatt(BluetoothGatt gatt)
    {
        this.gatt=gatt;
    }
    public BluetoothGatt get_gatt()
    {
        return gatt;
    }
    public void set_service(UUID uuid)
    {
        service=gatt.getService(uuid);
    }
    public BluetoothGattService get_service()
    {
        return service;
    }
    public void set_characteristic(UUID uuid)
    {
        characteristic=Include.get_characteristic(service,uuid,true);
    }
    public BluetoothGattCharacteristic get_characteristic()
    {
        return characteristic;
    }
}