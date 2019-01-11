package com.chain.iot.app.service;

import android.app.Service;
import android.app.Notification;
import com.chain.iot.app.R;
import com.chain.iot.app.include.DataMaster;
import com.chain.iot.app.include.Include;
import com.chain.iot.app.include.Interface;
import java.lang.Runnable;
import android.app.NotificationManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.util.Log;
import android.util.SparseArray;
import android.util.SparseBooleanArray;

import java.security.KeyPair;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.interfaces.RSAPublicKey;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class ServiceBle extends Service
{
    private UUID[] uuid_service=new UUID[1];
    private Handler handler;
    private Interface face;
    private boolean scan;//true-scan start,false-scan stop
    private boolean adv;//true-adv start,false-adv stop
    private KeyPair key_pair;
    //private RSAPublicKey key_pub;
    //private RSAPrivateCrtKey key_pri;
    /*
    //demo
    private final String[] address=new String[Include.DEMO_DEVICE];//device's address initial
    private final boolean[] condition=new boolean[Include.DEMO_DEVICE];//device's condition initial
    private int event;//EVENT_NONE,EVENT_OUTSIDE,EVENT_QUERY,EVENT_M2M,EVENT_ADV
    private int step;//step in each event
    private byte ret;//ret data
    */

    @Override
    public void onCreate()
    {
        super.onCreate();
        //
        if (Build.VERSION.SDK_INT>=Build.VERSION_CODES.JELLY_BEAN_MR2)//API>18
        {
            Notification.Builder builder = new Notification.Builder(this);
            builder.setSmallIcon(R.mipmap.ic_launcher);
            builder.setContentTitle("Ble");
            builder.setContentText("BleService");
            startForeground(Include.ID_SERVICEBLE, builder.build());//service run at foreground
        }
        else
            startForeground(Include.ID_SERVICEBLE, new Notification());
        //var
        handler=new Handler();//message handle to thread
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
            adv=true;
            break;
        case Include.DEBUG_PC:
            adv=false;
            break;
        case Include.DEBUG_C:
            adv=false;
            break;
        case Include.DEBUG_P:
            adv=false;
            break;
        case Include.DEBUG_BOTH:
            adv=false;
            break;
        }/*
            address[Include.DEVICE_REFRIGERATOR]="C8:FD:19:4F:94:71";//Refrigerator(d1)
            address[Include.DEVICE_GATELOCK]="C8:FD:19:4F:94:66";//GateLock(d2)
            address[Include.DEVICE_CAMERA]="C8:FD:19:4F:94:B6";//Camera(d3)
            address[Include.DEVICE_AIRCONDITIONER]="C8:FD:19:4F:91:7F";//AirConditioner(d4)
            address[Include.DEVICE_GARAGE]="C8:FD:19:4F:9A:36";//Garage(d5)
            address[Include.DEVICE_CARLOCK]="C8:FD:19:4F:9A:49";//CarLock(d6)
            condition[Include.DEVICE_REFRIGERATOR]=Include.SWITCH_ON;
            condition[Include.DEVICE_GATELOCK]=Include.SWITCH_OFF;
            condition[Include.DEVICE_CAMERA]=Include.SWITCH_OFF;
            condition[Include.DEVICE_AIRCONDITIONER]=Include.SWITCH_OFF;
            condition[Include.DEVICE_GARAGE]=Include.SWITCH_OFF;
            condition[Include.DEVICE_CARLOCK]=Include.SWITCH_ON;
            event=Include.EVENT_NONE;
            ret=Include.RET_QUERY;
            break;
        case Include.VERSION_MANUFACTURE:
            adv=false;
            break;
        }*/
        scan=false;
        uuid_service[0]=Include.ACCOUNT_SERVICE;
        key_pair=Include.gen_key(Include.RSAKEY_LENGTH);
        //key_pub=(RSAPublicKey)key_pair.getPublic();
        //key_pri=(RSAPrivateCrtKey)key_pair.getPrivate();
    }
    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        return new BinderBle();
    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        return START_STICKY;//return super.onStartCommand(intent, Service.START_FLAG_RETRY, startId);
    }
    @Override
    public void onDestroy()
    {
        NotificationManager manager;
        Intent intent;

        super.onDestroy();
        Log.i("itc","release:");
        //release master
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
        case Include.DEBUG_C:
        case Include.DEBUG_BOTH:
            for (DataMaster i : Include.get_master())
                if (i.get_status()!=Include.STATUS_DISCONNECT)
                    i.get_gatt().disconnect();
            Include.emp_master();
            if (scan)
            {
                scan=false;
                if (Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP)//API>21
                    Include.get_scanner().stopScan(callback_scan2);
                else
                    Include.get_adapter().stopLeScan(callback_scan);
                face.scan_stop();
            }
            break;
        }
        //release slave
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
        case Include.DEBUG_P:
        case Include.DEBUG_BOTH:
            if (adv)
            {
                adv=false;
                Include.get_slave().close_advertiser(callback_advertise);
                face.advertise_stop();
            }
            Include.get_slave().close_server();//close gatt server
            break;
        }
        //remove notify
        if (Build.VERSION.SDK_INT>=Build.VERSION_CODES.JELLY_BEAN_MR2)//API>18,remove notify when service is killed
        {
            manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
            manager.cancel(Include.ID_SERVICEBLE);
        }
        //keep service always reboot when killed
        intent=new Intent(getApplicationContext(), ServiceBle.class);
        startActivity(intent);
    }
    //set interface(gui)
    public void set_interface(Interface face)
    {
        this.face=face;
    }
    //init para(use under ble enabled)
    public void init()
    {
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
        case Include.DEBUG_P:
        case Include.DEBUG_BOTH:
            Include.get_slave().set_server(this, callback_gattserver);//enable gatt service
            Include.get_slave().set_advertiser();//set adv
            break;
        }
    }
    //initial(master/slave)
    public void initial()
    {
        handler.postDelayed(new Runnable()
        {
            @Override
            public void run()
            {
                Log.i("itc","initial:");
                initial();
                face.initial();
            }
        },Include.DELAY_WAIT);//initial start
        //adv switch
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
            if (!adv)
            {
                Log.i("itc","initial:open advertiser");
                adv=true;
                Include.get_slave().open_advertiser(callback_advertise);
                face.advertise_init();
            }
            else
            {
                Log.i("itc","initial:close advertiser");
                adv=false;
                Include.get_slave().close_advertiser(callback_advertise);
                face.advertise_stop();
            }
            break;
        case Include.DEBUG_P:
        case Include.DEBUG_BOTH:
            if (!adv)
            {
                Log.i("itc", "initial:open advertiser");
                adv = true;
                Include.get_slave().open_advertiser(callback_advertise);
                face.advertise_init();
            }
            break;
        }
        //master judgement
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
        case Include.DEBUG_C:
        case Include.DEBUG_BOTH:
            if (!adv && Include.get_number(Include.ROLE_CENTRAL)==Include.MAX_MASTER)//master exceed maximum
            {
                Log.i("itc", "initial:central maximum");
                return;
            }
            break;
        }
        //slave judgement
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
        case Include.DEBUG_P:
            if (adv && Include.get_number(Include.ROLE_PERIPHERAL)==Include.MAX_SLAVE)//slave connected
            {
                Log.i("itc", "initial:peri maximum");
                return;
            }
            break;
        }
        //goto scan
        switch(Include.DEBUG)
        {
        case Include.DEBUG_CP:
        case Include.DEBUG_PC:
        case Include.DEBUG_C:
            if (!adv)
            {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        Log.i("itc", "initial:scan");
                        scan();
                        face.initial();
                    }
                });
            }
            break;
        case Include.DEBUG_BOTH:
            //master:initial->scan
            handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    Log.i("itc","initial:scan");
                    scan();
                    face.initial();
                }
            });
            break;
        }
    }
    //scan(master):callback ScanCallback or BluetoothAdapter.LeScanCallback
    public void scan()
    {
        //goto scan start
        handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                List<ScanFilter> scanfilter;
                ScanSettings settings;
                byte i;

                Log.i("itc","scan:scan start");
                //delete not connected item
                Include.del_master();
                //scan
                if (Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP)//API>21
                {
                    //scan filter setting
                    scanfilter=new ArrayList<>();
                    scanfilter.add(new ScanFilter.Builder().setServiceUuid(new ParcelUuid(Include.ACCOUNT_SERVICE)).build());
                    settings=new ScanSettings.Builder().build();
                    Include.get_scanner().startScan(scanfilter,settings,callback_scan2);
                }
                else
                    Include.get_adapter().startLeScan(uuid_service,callback_scan);
                scan=true;
            }
        });
        //scan end, goto connect
        handler.postDelayed(new Runnable()
        {
            @Override
            public void run()
            {
                Log.i("itc","scan:scan stop");
                if (Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP)//API>21
                    Include.get_scanner().stopScan(callback_scan2);
                else
                    Include.get_adapter().stopLeScan(callback_scan);
                scan=false;
                for (DataMaster i : Include.get_master())
                    if (i.get_status()!=Include.STATUS_CONNECT && i.get_status()!=Include.STATUS_SERVICES)
                    {
                        connect(i.get_device().getAddress());
                        Log.i("itc","scan:connect "+i.get_device().getName());
                    }
                face.scan_stop();
            }
        },Include.DELAY_SCAN);
    }
    byte parse_manufacture(byte[] manufacture)
    {
        //according to len-tag-value stucture
        byte offset,len;

        offset=0;
        while(offset<manufacture.length-1)
        {
            len=manufacture[offset++];
            if (len==0)
                continue;
            if (manufacture[offset]!=0xff)//manufacture info
            {
                offset+=len;
                continue;
            }
            offset++;
            len--;
            //Check length
            if (len!=2+Include.MANUFACTURE_LEN)
            {
                offset+=len;
                continue;
            }
            break;
        }

        return offset;
    }
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private ScanCallback callback_scan2=new ScanCallback()
    {
        @Override
        public void onScanResult(int callbackType,ScanResult result)
        {
            DataMaster master;
            int rssi;
            byte offset;
            byte[] manufacture;
            int id;
            byte bluetooth,attach;

            super.onScanResult(callbackType, result);

            manufacture=result.getScanRecord().getManufacturerSpecificData(Include.ID_NONE);
            if (manufacture==null)
                manufacture=result.getScanRecord().getManufacturerSpecificData(Include.ID_NET);

            /*
            manufacture=result.getScanRecord().getBytes();
            offset=parse_manufacture(manufacture);


            id=manufacture[offset] | (manufacture[offset+1]<<8);
            bluetooth=manufacture[offset+2];
            attach=manufacture[offset+3];
            manufacture=result.getScanRecord().getManufacturerSpecificData(Include.ID_NONE);
            if (manufacture==null)
                manufacture=result.getScanRecord().getManufacturerSpecificData(Include.ID_NET);
            if (attach==Include.ATTACH_NET)
                return;*/
            if (manufacture[1]==Include.ATTACH_NET)
                return;
            //
            rssi=result.getRssi();
            Log.i("itc","scan callback2:add "+result.getDevice().getName()+" rssi:"+rssi);
            master=new DataMaster();
            master.set_status(Include.STATUS_DISCONNECT);
            master.set_device(result.getDevice());
            master.set_rssi(rssi);
            Include.add_master(master);
            face.scan_init(master);
        }
        @Override
        public void onBatchScanResults(List<ScanResult> results)
        {
            super.onBatchScanResults(results);
            //return batch scan list(including previous list)
            Log.i("itc","scan callback2:batch");
        }
        @Override
        public void onScanFailed(int errorCode)
        {
            super.onScanFailed(errorCode);
            Log.i("itc","scan callback2:failed "+errorCode);
            face.scan_error(errorCode);
        }
    };
    private BluetoothAdapter.LeScanCallback callback_scan=new BluetoothAdapter.LeScanCallback()
    {
        @Override
        public void onLeScan(BluetoothDevice bluetoothDevice,int rssi,byte[] scanRecord)
        {
            DataMaster master;

            Log.i("itc","scan callback:add "+bluetoothDevice.getName()+" rssi:"+rssi);
            master=new DataMaster();
            master.set_status(Include.STATUS_DISCONNECT);
            master.set_device(bluetoothDevice);
            master.set_rssi(rssi);
            Include.add_master(master);
            face.scan_init(master);
        }
    };
    //connect(master):callback BluetoothGattCallback, maximum 6 devices(synchronized only single connect for now)
    public void connect(final String address)//synchronized
    {
        handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                final BluetoothDevice device;
                final int index;

                Log.i("itc","connect:");
                device=Include.get_adapter().getRemoteDevice(address);
                if (device==null)
                    return;
                index=Include.get_index(address);
                Include.get_master().get(index).set_gatt(device.connectGatt(ServiceBle.this, false, callback_gatt));
                face.connect_init(address);
            }
        });
    }
    //callback_gatt(master)
    private final BluetoothGattCallback callback_gatt=new BluetoothGattCallback()
    {
        @Override
        public void onConnectionStateChange(final BluetoothGatt gatt,final int status,final int newState)
        {
            //use in connection state change : newState-current state, status-previous state
            final String address;
            final int index;

            super.onConnectionStateChange(gatt,status,newState);
            //
            address=Include.get_address(gatt);
            index=Include.get_index(gatt);
            switch(newState)
            {
            case BluetoothProfile.STATE_DISCONNECTED://master disconnected
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        Log.i("itc","connect callback:disconnected "+gatt.getDevice().getName());
                        Include.get_master().get(index).set_status(Include.STATUS_DISCONNECT);
                        gatt.close();
                        face.status_disconnect(address);
                    }
                });
                break;
            case BluetoothProfile.STATE_CONNECTING://master connecting
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        Log.i("itc","connect callback:connecting "+gatt.getDevice().getName());
                        Include.get_master().get(index).set_status(Include.STATUS_CONNECTING);
                        face.status_connecting(address);
                    }
                });
                break;
            case BluetoothProfile.STATE_CONNECTED://master connected
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        Log.i("itc","connect callback:connected "+gatt.getDevice().getName());
                        gatt.discoverServices();//find slave's services
                        Include.get_master().get(index).set_status(Include.STATUS_CONNECT);
                        face.status_connect(address);
                        face.connect_success(address);
                    }
                });
                break;
            case BluetoothProfile.STATE_DISCONNECTING://master disconnecting
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        Log.i("itc","connect callback:disconnecting "+gatt.getDevice().getName());
                        Include.get_master().get(index).set_status(Include.STATUS_DISCONNECTING);
                        face.status_disconnecting(address);
                    }
                });
                break;
            default://exception state
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        if (gatt!=null)
                            gatt.disconnect();
                        Log.i("itc", "connect callback:connect failed " + newState+" "+gatt.getDevice().getName());
                        face.connect_error(address,newState);
                    }
                });
                break;
            }
        }
        @Override
        public void onServicesDiscovered(final BluetoothGatt gatt,final int status)
        {
            final String address1;
            final int index;

            super.onServicesDiscovered(gatt, status);
            //
            index=Include.get_index(gatt);
            address1=Include.get_address(gatt);
            Include.get_master().get(index).set_service(Include.ACCOUNT_SERVICE);
            Include.get_master().get(index).set_characteristic(Include.ACCOUNT_CHARACTERISTIC);
            gatt.setCharacteristicNotification(Include.get_master().get(index).get_characteristic(),true);//enable notification
            //characteristic=Include.get_characteristic(service,Include.ACCOUNT_CHARACTERISTIC,false);
            switch(status)
            {
            case BluetoothGatt.GATT_SUCCESS:
                handler.post(new Runnable()
                {
                     @Override
                     public void run()
                     {
                         Log.i("itc", "connect callback:service discovered "+gatt.getDevice().getName());
                         Include.set_status(Include.ROLE_CENTRAL,index,Include.STATUS_SERVICES);
                         face.service_success(address1);
                     }
                 });
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        boolean flag;
                        final byte[] buffer;

                        flag=true;
                        buffer=Include.get_master().get(index).get_buffer();
                        switch(Include.DEBUG)
                        {
                        case Include.DEBUG_BOTH:
                            buffer[0]=Include.CMD_CLOCK;
                            if (Include.get_master().get(index).get_condition())
                            {
                                buffer[1]=Include.FALSE;
                                Include.get_master().get(index).set_condition(Include.SWITCH_OFF);
                            }
                            else
                            {
                                buffer[1]=Include.TRUE;
                                Include.get_master().get(index).set_condition(Include.SWITCH_ON);
                            }
                            break;
                        default:
                            buffer[0]='A';
                            Log.i("itc","connect callback:transfer "+(char)buffer[0]+" "+gatt.getDevice().getName());
                            face.characteristic_w_s(address1,buffer);
                            break;
                        }
                        if (flag)
                            transfer(gatt,Include.get_master().get(index).get_characteristic(),buffer);
                    }
                });
                break;
            default:
                handler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        Log.i("itc", "connect callback:service failed "+status+" "+gatt.getDevice().getName());
                        face.connect_error(address1, status);
                    }
                });
                break;
            }
        }
        @Override
        public void onCharacteristicRead(final BluetoothGatt gatt,BluetoothGattCharacteristic characteristic,int status)
        {
            super.onCharacteristicRead(gatt, characteristic, status);
            //master read from slave's characteristic(after slave's response)
            Log.i("itc","connect callback:char read "+gatt.getDevice().getName());
            switch(status)
            {
            case BluetoothGatt.GATT_SUCCESS:
                break;
            default:
                break;
            }
        }
        @Override
        public void onCharacteristicWrite(final BluetoothGatt gatt,final BluetoothGattCharacteristic characteristic,int status)
        {
            final String address1;
            final int index;
            final byte[] buffer;

            super.onCharacteristicWrite(gatt, characteristic, status);
            //master write to slave's characteristic(after slave's response)
            address1=Include.get_address(gatt);
            index=Include.get_index(gatt);
            buffer=Include.get_master().get(index).get_buffer();
            Log.i("itc","connect callback:char write "+gatt.getDevice().getName());
            switch(status)
            {
            case BluetoothGatt.GATT_SUCCESS:
                if (!characteristic.getUuid().equals(Include.ACCOUNT_CHARACTERISTIC))
                    return;
                switch(Include.DEBUG)
                {
                case Include.DEBUG_BOTH:
                    break;
                default:
                    if (buffer[0]=='C')
                    {
                        handler.postDelayed(new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                Log.i("itc", "connect callback:char end "+gatt.getDevice().getName());
                                gatt.disconnect();
                                //release();
                            }
                        }, Include.DELAY_GAP);
                    }
                    else
                    {
                        handler.postDelayed(new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                buffer[0]++;
                                Log.i("itc", "connect callback:transfer " + (char) buffer[0]+" "+gatt.getDevice().getName());
                                face.characteristic_w_s(address1, buffer);
                                transfer(gatt, characteristic, buffer);
                            }
                        }, Include.DELAY_GAP);
                    }
                    break;
                }
                break;
            default:
                break;
            }
        }
        @Override
        public void onCharacteristicChanged(final BluetoothGatt gatt,BluetoothGattCharacteristic characteristic)
        {
            final byte[] data;
            int i,len;
            String str;

            super.onCharacteristicChanged(gatt, characteristic);
            //slave notify master of characteristic
            Log.i("itc","connect callback:notify "+gatt.getDevice().getName());
            data=characteristic.getValue();
            len=characteristic.getValue().length;
            str=gatt.getDevice().getName()+"(s)("+gatt.getDevice().getAddress()+"):len="+len;
            for (i=0;i<len;i++)
                str=str.concat(","+data[i]);
            Log.i("itc",str);
        }
        @Override
        public void onDescriptorRead(final BluetoothGatt gatt,BluetoothGattDescriptor descriptor,int status)
        {
            final String address;

            super.onDescriptorRead(gatt, descriptor, status);
            //master read from slave's descriptor
            address=Include.get_address(gatt);
            face.descriptor_r_s(address);
        }
        @Override
        public void onDescriptorWrite(final BluetoothGatt gatt,BluetoothGattDescriptor descriptor,int status)
        {
            final String address;

            super.onDescriptorWrite(gatt, descriptor, status);
            //master write to slave's descriptor
            address=Include.get_address(gatt);
            face.descriptor_w_s(address,descriptor.getValue());
        }
        @Override
        public void onReadRemoteRssi(final BluetoothGatt gatt, int rssi, int status)
        {/*
            final int index;
            final int i;
            final byte[] buffer;*/

            super.onReadRemoteRssi(gatt,rssi,status);/*
            //
            index=Include.get_index(gatt);
            if (!Include.is_connect(Include.ROLE_CENTRAL,index))
                return;
            Log.i("itc",""+gatt.getDevice().getName()+" rssi:"+rssi);
            Log.i("itc","step:"+step+" address:"+Include.get_index(address,gatt.getDevice().getAddress()));
            Include.get_master().get(index).set_rssi(rssi);
            buffer=Include.get_master().get(index).get_buffer();*/
        }
    };
    //transfer data(master)
    public void transfer(final BluetoothGatt gatt,BluetoothGattCharacteristic characteristic,byte[] data)
    {
        Log.i("itc","transfer: "+gatt.getDevice().getName());
        if (characteristic==null)
            return;
        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);//WRITE_TYPE_DEFAULT-need requesting acknoledgement by slave,WRITE_TYPE_NO_RESPONSE-no response by slave,WRITE_TYPE_SIGNED-need authentication signature included
        characteristic.setValue(data);
        gatt.writeCharacteristic(characteristic);
    }
    //callback_adv(slave)
    private AdvertiseCallback callback_advertise=new AdvertiseCallback()
    {
        @Override
        public void onStartFailure(int errorCode)
        {
            super.onStartFailure(errorCode);
            //
            Log.i("itc","peri callback:adv failed "+errorCode);
            face.advertise_error(errorCode);
        }
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect)
        {
            super.onStartSuccess(settingsInEffect);
            //
            Log.i("itc","peri callback:adv success");
            Include.get_slave().set_status(Include.STATUS_ADVERTISING);
            face.advertise_success();
        }
    };
    //callback_gattserver(slave)
    private BluetoothGattServerCallback callback_gattserver=new BluetoothGattServerCallback()
    {
        @Override
        public void onConnectionStateChange(BluetoothDevice device,int status,int newState)
        {
            //use in connection state change : newState-current state, status-previous state
            boolean flag;

            super.onConnectionStateChange(device, status, newState);
            //
            switch (newState)
            {
            case BluetoothProfile.STATE_DISCONNECTED://disconnected
                Log.i("itc", "peri callback:disconnected "+device.getName());
                Include.get_slave().set_status(Include.STATUS_DISCONNECT);
                face.status_disconnect();
                switch(Include.DEBUG)
                {
                case Include.DEBUG_BOTH:
                    adv=true;
                    Include.get_slave().open_advertiser(callback_advertise);
                    face.advertise_init();
                    break;
                default:
                    break;
                }
                break;
                /*
                //handler.removeCallbacksAndMessages(null);
                flag=true;
                switch(Include.VERSION)
                {
                case Include.VERSION_MODULE:
                    switch(Include.DEBUG)
                    {
                    case Include.DEBUG_BOTH:
                        flag=false;
                        adv=true;
                        Include.get_slave().open_advertiser(callback_advertise);
                        face.advertise_init();
                        break;
                    default:
                        break;
                    }
                    break;
                case Include.VERSION_DEMO:
                    switch(event)
                    {
                    case Include.EVENT_NONE:
                        flag=false;
                        break;
                    case Include.EVENT_OUTSIDE:
                        break;
                    case Include.EVENT_QUERY:
                        break;
                    case Include.EVENT_M2M:
                        switch(step)
                        {
                        case Include.STEP_NONE:
                            step++;
                            break;
                        case Include.STEP_1:
                            step=Include.STEP_NONE;
                            break;
                        }
                        break;
                    }
                    break;
                case Include.VERSION_MANUFACTURE:
                    break;
                }
                if (flag)
                {
                    handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            initial();
                        }
                    });
                }
                break;*/
            case BluetoothProfile.STATE_CONNECTING://connecting
                Log.i("itc", "peri callback:connecting "+device.getName());
                Include.get_slave().set_status(Include.STATUS_CONNECTING);
                face.status_connecting();
                break;
            case BluetoothProfile.STATE_CONNECTED://connected
                Log.i("itc", "peri callback:connected "+device.getName());
                Include.get_slave().set_status(Include.STATUS_CONNECT);
                face.status_connect();
                switch(Include.DEBUG)
                {
                case Include.DEBUG_BOTH:
                    adv=false;
                    Include.get_slave().close_advertiser(callback_advertise);
                    face.advertise_stop();
                    break;
                default:
                    break;
                }
                break;
            case BluetoothProfile.STATE_DISCONNECTING://disconnecting
                Log.i("itc", "peri callback:disconnecting "+device.getName());
                Include.get_slave().set_status(Include.STATUS_DISCONNECTING);
                face.status_disconnecting();
                break;
            }
        }
        @Override
        public void onServiceAdded(int status,BluetoothGattService service)
        {
            super.onServiceAdded(status, service);
            //add service finished
            switch(status)
            {
            case BluetoothGatt.GATT_SUCCESS:
                Log.i("itc","peri callback:service add");
                break;
            default:
                Log.i("itc","peri callback:service error");
                break;
            }
        }
        @Override
        public void onCharacteristicWriteRequest(final BluetoothDevice device, int requestId, BluetoothGattCharacteristic characteristic, boolean preparedWrite, boolean responseNeeded, int offset, final byte[] value)
        {
            super.onCharacteristicWriteRequest(device, requestId, characteristic, preparedWrite, responseNeeded, offset, value);
            //slave accept master's characteristic write request
            if (!characteristic.getUuid().equals(Include.ACCOUNT_CHARACTERISTIC))
                return;
            Include.get_slave().get_server().sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, value);


            int i,len;
            String str;

            len=value.length;
            str=device.getName()+"(m)("+device.getAddress()+"):len="+len;
            for (i=0;i<len;i++)
                str=str.concat(","+value[i]);
            Log.i("itc",str);

            /*
            handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    switch(Include.DEBUG)
                    {
                    case Include.DEBUG_BOTH:
                        Log.i("itc","peri callback:char write "+value[0]);
                        switch(value[0])
                        {
                        case Include.RET_CLOCK:
                            break;
                        case Include.RET_EMERGENCY:
                            Log.i("itc","emergency "+value[1]);
                            //adv=false;
                            //Include.get_slave().close_advertiser(callback_advertise);
                            //face.advertise_stop();
                            break;
                        }
                        break;
                    default:
                        Log.i("itc", "peri callback:char write "+(char)value[0]);
                        face.characteristic_w_s(value);
                        break;
                    }
                }
            });
            */
        }
        @Override
        public void onCharacteristicReadRequest(BluetoothDevice device, int requestId, int offset, BluetoothGattCharacteristic characteristic)
        {
            byte[] data;

            super.onCharacteristicReadRequest(device, requestId, offset, characteristic);
            //slave accept/respond master's characteristic read request
            if (!characteristic.getUuid().equals(Include.ACCOUNT_CHARACTERISTIC))
                return;
            Include.get_slave().get_server().sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, "".getBytes());
            data=characteristic.getValue();
            Log.i("itc", "peri callback:char read"+data[0]);
            face.characteristic_r_s();
        }
        @Override
        public void onDescriptorWriteRequest(BluetoothDevice device, int requestId, BluetoothGattDescriptor descriptor, boolean preparedWrite, boolean responseNeeded, int offset, byte[] value)
        {
            super.onDescriptorWriteRequest(device, requestId, descriptor, preparedWrite, responseNeeded, offset, value);
            //slave accept master's descriptor write request
            if (!descriptor.getUuid().equals(Include.ACCOUNT_DESCRIPTOR))
                return;
            Log.i("itc", "peri callback:desc write");
            Include.get_slave().get_server().sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, value);
            face.descriptor_w_s(value);
        }
        @Override
        public void onDescriptorReadRequest(BluetoothDevice device, int requestId, int offset, BluetoothGattDescriptor descriptor)
        {
            super.onDescriptorReadRequest(device, requestId, offset, descriptor);
            //slave accept/respond master's descriptor read request
            if (!descriptor.getUuid().equals(Include.ACCOUNT_DESCRIPTOR))
                return;
            Log.i("itc", "peri callback:desc read");
            Include.get_slave().get_server().sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            face.descriptor_r_s();
        }
        @Override
        public void onNotificationSent(BluetoothDevice device, int status)
        {
            super.onNotificationSent(device, status);
            //slave notify master's
            Log.i("itc", "peri callback:notify");
        }
    };
    //service binder
    public class BinderBle extends Binder
    {
        //get current service's instance
        public ServiceBle get_service()
        {
            return ServiceBle.this;
        }
    }
    //preprocess(demo)
    public void preprocess(int event)
    {
        int i;

        //this.event=event;
        //step=Include.STEP_NONE;
        handler.removeCallbacksAndMessages(null);
        /*
        //disconnect device3/4
        switch(event)
        {
        case Include.EVENT_M2M:
            i=Include.get_index(address[Include.DEVICE_CAMERA]);
            if (i!=Include.get_master().size())
                Include.get_master().get(i).get_gatt().disconnect();
            i=Include.get_index(address[Include.DEVICE_AIRCONDITIONER]);
            if (i!=Include.get_master().size())
                Include.get_master().get(i).get_gatt().disconnect();
            break;
        default:
            break;
        }*/
        initial();
    }/*
    //set/get event
    public void set_event(int event)
    {
        this.event=event;
    }
    public int get_event()
    {
        return event;
    }
    //get ret
    public byte get_ret()
    {
        return ret;
    }
    */
    //private function
    private void status_change(final BluetoothGatt gatt,int newState)
    {
        int index;

        index=Include.get_index(gatt);
        switch(newState)
        {
        case BluetoothProfile.STATE_CONNECTING:
            Include.get_master().get(index).set_status(Include.STATUS_CONNECTING);
            break;
        case BluetoothProfile.STATE_CONNECTED:
            Include.get_master().get(index).set_status(Include.STATUS_CONNECT);
            break;
        case BluetoothProfile.STATE_DISCONNECTING:
            Include.get_master().get(index).set_status(Include.STATUS_DISCONNECTING);
            break;
        case BluetoothProfile.STATE_DISCONNECTED:
            Include.get_master().get(index).set_status(Include.STATUS_DISCONNECT);
            break;
        }
    }
}
