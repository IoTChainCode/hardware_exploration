package com.chain.iot.app.include;

import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.os.ParcelUuid;

public class DataSlave extends Data
{
    private BluetoothGattServer server;//bluetooth gatt server
    private BluetoothGattService service;//bluetooth gatt service
    private BluetoothGattCharacteristic characteristic;//bluetooth gatt characteristic
    private BluetoothGattDescriptor descriptor;//bluetooth gatt descriptor
    private BluetoothLeAdvertiser advertiser;//bluetooth advertiser
    private AdvertiseSettings settings;
    private AdvertiseData data;
    //
    public DataSlave()
    {
        super();
        //
        set_status(Include.STATUS_DISCONNECT);
        //create gatt service
        service=new BluetoothGattService(Include.ACCOUNT_SERVICE,BluetoothGattService.SERVICE_TYPE_PRIMARY);
        //create gatt characteristic
        characteristic=new BluetoothGattCharacteristic(Include.ACCOUNT_CHARACTERISTIC, BluetoothGattCharacteristic.PROPERTY_READ | BluetoothGattCharacteristic.PROPERTY_WRITE | BluetoothGattCharacteristic.PROPERTY_NOTIFY,BluetoothGattCharacteristic.PERMISSION_READ | BluetoothGattCharacteristic.PERMISSION_WRITE);
        //create gatt descriptor(optional)
        descriptor=new BluetoothGattDescriptor(Include.ACCOUNT_DESCRIPTOR,BluetoothGattDescriptor.PERMISSION_READ | BluetoothGattCharacteristic.PERMISSION_WRITE);
        characteristic.addDescriptor(descriptor);
        service.addCharacteristic(characteristic);
        //adv setting
        settings=new AdvertiseSettings.Builder()
                .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_BALANCED)//adv mode.ADVERTISE_MODE_LOW_POWER,ADVERTISE_MODE_BALANCED,ADVERTISE_MODE_LOW_LATENCY
                .setConnectable(true)//device connectable.enable,disable
                .setTimeout(0)//adv time limit.0-disable time limit,max-180s
                .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_MEDIUM)//adv tx power level.ADVERTISE_TX_POWER_ULTRA_LOW,ADVERTISE_TX_POWER_LOW,ADVERTISE_TX_POWER_MEDIUM,ADVERTISE_TX_POWER_HIGH
                .build();
        //adv data
        data=new AdvertiseData.Builder()
                .setIncludeDeviceName(true)//device name included.enable,disable
                .setIncludeTxPowerLevel(true)//tx power level included.enable,disable
                .addServiceUuid(new ParcelUuid(Include.ACCOUNT_SERVICE))//adv services owned
                .addManufacturerData(Include.ID_NET,Include.get_manufacture())//manufacture data
                .build();
    }
    public void set_server(Context context, BluetoothGattServerCallback callback)
    {
        server=Include.get_manager().openGattServer(context,callback);
        server.addService(service);//add gatt service into gatt server
    }
    public BluetoothGattServer get_server()
    {
        return server;
    }
    public void close_server()
    {
        server.clearServices();
        server.close();
    }
    public void set_service(BluetoothGattService service)
    {
        this.service=service;
    }
    public BluetoothGattService get_service()
    {
        return service;
    }
    public void set_characteristic(BluetoothGattCharacteristic characteristic)
    {
        this.characteristic=characteristic;
    }
    public BluetoothGattCharacteristic get_characteristic()
    {
        return characteristic;
    }
    public void set_descriptor(BluetoothGattDescriptor descriptor)
    {
        this.descriptor=descriptor;
    }
    public BluetoothGattDescriptor get_descriptor()
    {
        return descriptor;
    }
    public void set_advertiser()
    {
        advertiser=Include.get_adapter().getBluetoothLeAdvertiser();
    }
    public BluetoothLeAdvertiser get_advertiser()
    {
        return advertiser;
    }
    public void open_advertiser(AdvertiseCallback callback)
    {
        advertiser.startAdvertising(settings,data,callback);
    }
    public void close_advertiser(AdvertiseCallback callback)
    {
        advertiser.stopAdvertising(callback);
    }
    public void set_settings(AdvertiseSettings settings)
    {
        this.settings=settings;
    }
    public AdvertiseSettings get_settings()
    {
        return settings;
    }
    public void set_data(AdvertiseData data)
    {
        this.data=data;
    }
    public AdvertiseData get_data()
    {
        return data;
    }
}