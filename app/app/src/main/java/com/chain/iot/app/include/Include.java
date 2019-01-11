package com.chain.iot.app.include;

import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v4.content.ContextCompat;
import javax.crypto.Cipher;

import static java.lang.StrictMath.abs;

public class Include
{
    //macro(debug)
    public static final int DEBUG_CP=0x00;//central-peripheral-central-peripheral-...
    public static final int DEBUG_PC=0x01;//peripheral-central-peripheral-central-...
    public static final int DEBUG_C=0x02;//central always
    public static final int DEBUG_P=0x03;//peripheral always
    public static final int DEBUG_BOTH=0x04;//both central/peripheral
    public static final int DEBUG=DEBUG_BOTH;
    //delay
    public static final int DELAY_GAP=500;
    public static final int DELAY_SCAN=6000;
    public static final int DELAY_WAIT=20000;
    //max
    public static final int MAX_MASTER=6;//max master number
    public static final int MAX_SLAVE=1;//max slave number
    //role
    public static final byte ROLE_PERIPHERAL=0x00;
    public static final byte ROLE_CENTRAL=0x01;
    //status
    public static final byte STATUS_SCANNING=0x00;
    public static final byte STATUS_CONNECT=0x01;
    public static final byte STATUS_CONNECTING=0x02;
    public static final byte STATUS_DISCONNECTING=0x03;
    public static final byte STATUS_DISCONNECT=0x04;
    public static final byte STATUS_SERVICES=0x05;
    public static final byte STATUS_CHARACTERISTIC=0x06;//now same as STATUS_SERVICES, assumed char is exist(as filter by address or manufacture_id)
    public static final byte STATUS_ADVERTISING=0x07;
    //id
    public static final int ID_SERVICEBLE=0x0100;
    public static final int ID_NONE=0x1234;
    public static final int ID_NET=0x5678;
    //request code
    public static final byte REQUEST_CODE_BLE=0x00;
    public static final byte REQUEST_CODE_GPS=0x01;
    //uuid
    public static final UUID ACCOUNT_SERVICE=UUID.fromString("0000fff0-0000-1000-8000-00805f9b34fb");//service uuid
    public static final UUID ACCOUNT_CHARACTERISTIC=UUID.fromString("0000fff6-0000-1000-8000-00805f9b34fb");//characteristic uuid
    public static final UUID ACCOUNT_DESCRIPTOR=UUID.fromString("00000000-0000-1000-8000-00805f9b34fb");//descriptor uuid. not used,should not be null
    //manufacture
    public static final int MANUFACTURE_LEN=0x02;
    //ble length
    public static final int BLE_LENGTH=20;//byte
    //rsakey length
    public static final int RSAKEY_LENGTH=1024;//bit
    //address length
    public static final int ADDRESS_LENGTH=6;//byte
    //bluetooth version
    public static final byte BLUETOOTH_V10=0x01;
    public static final byte BLUETOOTH_V11=0x02;
    public static final byte BLUETOOTH_V12=0x03;
    public static final byte BLUETOOTH_V20=0x04;
    public static final byte BLUETOOTH_V21=0x05;
    public static final byte BLUETOOTH_V30=0x06;
    public static final byte BLUETOOTH_V40=0x07;
    public static final byte BLUETOOTH_V41=0x08;
    public static final byte BLUETOOTH_V42=0x09;
    public static final byte BLUETOOTH_V50=0x0a;
    //attach
    public static final byte ATTACH_NONE=0x01;
    public static final byte ATTACH_NET=0x02;
    /*
    //device
    public static final int DEVICE_REFRIGERATOR=0x00;
    public static final int DEVICE_GATELOCK=0x01;
    public static final int DEVICE_CAMERA=0x02;
    public static final int DEVICE_AIRCONDITION=0x03;
    public static final int DEVICE_GARAGE=0x04;
    public static final int DEVICE_CARLOCK=0x05;
    */
    //switch
    public static final boolean SWITCH_ON=true;
    public static final boolean SWITCH_OFF=false;
    //TRUE/FALSE
    public static final byte FALSE=0x00;
    public static final byte TRUE=0x01;
    //user cmd
    public static final int CMD_QUERY=0x00;
    public static final int CMD_SWITCH=0x01;
    public static final int CMD_M2M=0x02;
    public static final int CMD_CLOCK=0x03;
    public static final int CMD_ADV=0x04;
    //user ret
    public static final int RET_QUERY=0x00;
    public static final int RET_M2M=0x01;
    public static final int RET_FIRE=0x02;
    public static final int RET_CLOCK=0x03;
    public static final int RET_EMERGENCY=0x04;/*
    //event
    public static final int EVENT_NONE=0x00;
    public static final int EVENT_OUTSIDE=0x01;
    public static final int EVENT_QUERY=0x02;
    public static final int EVENT_M2M=0x03;
    public static final int EVENT_ADV=0x04;

    //step
    public static final int STEP_NONE=0x00;
    public static final int STEP_1=0x01;//device5 on(outside),device1 query(query),device1 ret_m2m(m2m)
    public static final int STEP_2=0x02;//device6 off(outside),device2 query(query)
    public static final int STEP_3=0x03;//device5 off(outside),device3 query(query)
    public static final int STEP_4=0x04;//device2 on(outside),device4 query(query)

    //distance(meter)
    public static final double DISTANCE_GATELOCK=0.3;
    public static final double DISTANCE_GARAGE=0.3;
    public static final double DISTANCE_CARLOCK=1.5;
    */
    //var
    private static Include include;
    private static BluetoothManager manager;
    private static BluetoothAdapter adapter;
    private static BluetoothLeScanner scanner;
    private static List<DataMaster> master;
    private static DataSlave slave;
    private static byte[] manufacture_data=new byte[MANUFACTURE_LEN];

    //init
    public static void init(Context context)
    {
        synchronized(Include.class)
        {
            include=new Include(context);
        }
    }
    //get manager
    public static BluetoothManager get_manager()
    {
        return manager;
    }
    //get adapter
    public static BluetoothAdapter get_adapter()
    {
        return adapter;
    }
    //get scanner
    public static BluetoothLeScanner get_scanner()
    {
        return scanner;
    }
    //get manufacture_data
    public static byte[] get_manufacture()
    {
        return manufacture_data;
    }
    //set/get status
    public static void set_status(byte role,int index,byte status)
    {
        if (role==ROLE_CENTRAL)
            master.get(index).set_status(status);
        else
            slave.set_status(status);
    }
    public static byte get_status(byte role,int index)
    {
        return role==ROLE_CENTRAL ? master.get(index).get_status() : slave.get_status();
    }
    //Judge whether ble connected.false-not connect,true-connected
    public static boolean is_connect(byte role,int index)
    {
        return role==ROLE_CENTRAL ? master.get(index).get_status()==STATUS_CONNECT || master.get(index).get_status()==STATUS_SERVICES : slave.get_status()==STATUS_CONNECT;
    }
    //get master
    public static List<DataMaster> get_master()
    {
        return master;
    }
    //get slave
    public static DataSlave get_slave()
    {
        return slave;
    }
    //add master
    public static void add_master(DataMaster data)
    {
        boolean flag;

        if (master!=null && master.size()>0)
        {
            flag=false;
            for (DataMaster i : master)
                if (i.get_device().getAddress().equals(data.get_device().getAddress()))
                {
                    flag=true;
                    break;
                }
            if (!flag)
                master.add(data);
        }
        else
            master.add(data);
    }
    //delete master
    public static void del_master()
    {
        if (master==null)
            return;
        for (int i=0;i<master.size();i++)
            if (master.get(i).get_status()!=STATUS_CONNECT && master.get(i).get_status()!=STATUS_SERVICES)
                master.remove(i);
    }
    //empty master
    public static void emp_master()
    {
        if (master!=null && master.size()>0)
            master.clear();
    }
    //set/get gatt services
    public static void set_services(int index,List<BluetoothGattService> services)
    {
        List<BluetoothGattService> list;

        list=master.get(index).get_gatt().getServices();
        if (list!=null && list.size()>0)
            list.clear();
        list.addAll(services);
    }
    public static List<BluetoothGattService> get_services(int index)
    {
        return master.get(index).get_gatt().getServices();
    }
    //get gatt service
    public static BluetoothGattService get_service(int index,UUID uuid)
    {
        return master.get(index).get_gatt().getService(uuid);
    }
    //get index
    public static int get_index(BluetoothGatt gatt)
    {
        int i;

        for (i=0;i<master.size();i++)
            if (master.get(i).get_gatt().getDevice().equals(gatt.getDevice()))
                break;

        return i;
    }
    public static int get_index(String address)
    {
        int i;

        for (i=0;i<master.size();i++)
            if (master.get(i).get_device().getAddress().equals(address))
                break;

        return i;
    }/*
    public static int get_index(String[] address,String string)
    {
        int i;

        for (i=0;i<DEMO_DEVICE;i++)
            if (address[i].equals(string))
                break;

        return i;
    }*/
    //get characteristic(true-filter notify or indicate,false-no filter)
    public static BluetoothGattCharacteristic get_characteristic(BluetoothGattService service,UUID uuid,boolean flag)
    {
        List<BluetoothGattCharacteristic> characteristics;

        characteristics=service.getCharacteristics();
        for (BluetoothGattCharacteristic i : characteristics)
        {
            if ((!flag ||
                 ((i.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY)!=0 ||
                 (i.getProperties() & BluetoothGattCharacteristic.PROPERTY_INDICATE)!=0))
                    && uuid.equals(i.getUuid()))
                return i;
        }

        return null;
    }
    //get connect device's MAC address
    public static String get_address(BluetoothGatt gatt)
    {
        BluetoothDevice device;

        if (gatt==null)
            return "";
        device=gatt.getDevice();
        if (device==null)
            return "";

        return device.getAddress();
    }
    //device:whether support ble
    public static boolean ble_support(Context context)
    {
        return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
    }
    //device:whether support ble slave
    public static boolean ble_slave()
    {
        return true;
        //return adapter.isMultipleAdvertisementSupported();//should use after ble enabled
    }
    //device:whether enable ble
    public static boolean ble_enable()
    {
        return adapter!=null && adapter.isEnabled();
    }
    //device:whether enable global gps
    public static boolean gps_global(final Context context)
    {
        LocationManager manager;

        manager=(LocationManager)context.getSystemService(Context.LOCATION_SERVICE);

        return manager.isProviderEnabled(LocationManager.GPS_PROVIDER) || manager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
    }
    //device:whether enable app gps
    @RequiresApi(api = Build.VERSION_CODES.M)
    public static boolean gps_app(Context context)
    {
        return ContextCompat.checkSelfPermission(context,Manifest.permission.ACCESS_COARSE_LOCATION)==PackageManager.PERMISSION_GRANTED;
    }
    //gen rsa key
    public static KeyPair gen_key(int length)
    {
        //length:key length,512~2048bit
        try
        {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
            SecureRandom rand = new SecureRandom();
            rand.setSeed("anything".getBytes());
            kpg.initialize(length,rand);
            return kpg.genKeyPair();
        }
        catch (NoSuchAlgorithmException e)
        {
            e.printStackTrace();
            return null;
        }
    }
    //rsa enc
    //data:plain text
    //key:public key
    public static byte[] rsa_enc(byte[] data, byte[] key) throws Exception
    {
        //get key
        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(key);
        KeyFactory kf = KeyFactory.getInstance("RSA");
        PublicKey keyPublic = kf.generatePublic(keySpec);
        //enc
        Cipher cp = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cp.init(Cipher.ENCRYPT_MODE, keyPublic);
        return cp.doFinal(data);
    }
    //rsa dec
    //data:cipher text
    //key:private key
    public static byte[] rsa_dec(byte[] data, byte[] key) throws Exception
    {
        //get key
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(key);
        KeyFactory kf = KeyFactory.getInstance("RSA");
        PrivateKey keyPrivate = kf.generatePrivate(keySpec);
        //dec
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(Cipher.DECRYPT_MODE, keyPrivate);
        return cipher.doFinal(data);
    }
    //get distance by rssi
    public static double get_distance(int rssi)
    {
        //distance(meter)=10^((abs(RSSI)-A)/(10*n)),RSSI-signal strength(negtive),A-signal strength between 1 meter points(A=59),n-attenuation factor(n=2.0~2.5)
        return Math.pow(10.0,(abs(rssi)-59)/(10*2.0));
    }
    //get number
    public static int get_number(byte role)
    {
        int number;

        number=0;
        switch(role)
        {
        case ROLE_CENTRAL:
            for (DataMaster i : master)
                if (i.get_status()==Include.STATUS_CONNECT || i.get_status()==Include.STATUS_SERVICES)
                    number++;
            break;
        case ROLE_PERIPHERAL:
            if (slave.get_status()==Include.STATUS_CONNECT)
                number++;
            break;
        }

        return number;
    }
    //private function
    private Include(Context context)
    {
        byte i;

        if (android.os.Build.VERSION.SDK_INT>=android.os.Build.VERSION_CODES.JELLY_BEAN_MR2)//API>18
        {
            manager=(BluetoothManager)context.getSystemService(Context.BLUETOOTH_SERVICE);
            adapter=manager.getAdapter();
        }
        else
        {
            manager=null;
            adapter=BluetoothAdapter.getDefaultAdapter();
        }
        if (Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP)//API>21
            scanner=adapter.getBluetoothLeScanner();
        else
            scanner=null;
        manufacture_data[0]=Include.BLUETOOTH_V42;//should use api
        manufacture_data[1]=Include.ATTACH_NET;
        master=new ArrayList<>();
        slave=new DataSlave();
    }
}