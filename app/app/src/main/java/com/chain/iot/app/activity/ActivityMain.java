package com.chain.iot.app.activity;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import com.chain.iot.app.R;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.OrientationHelper;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import com.chain.iot.app.include.Data;
import com.chain.iot.app.include.DataMaster;
import com.chain.iot.app.include.DataSlave;
import com.chain.iot.app.include.Interface;
import com.chain.iot.app.include.AdapterBle;
import com.chain.iot.app.include.Include;
import com.chain.iot.app.service.ServiceBle;
import android.content.Intent;
import android.widget.Toast;

public class ActivityMain extends ActivityBase
{
    //view
    private Button button;
    private TextView text_view;
    private SwipeRefreshLayout swipe_refresh_layout;
    private RecyclerView recycler_view;
    //data
    private AdapterBle adapter_ble;
    private Intent intent;
    //demo
    private Button button_outside,button_query,button_m2m,button_adv;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        LinearLayoutManager layout_manager;

        super.onCreate(savedInstanceState);/*
        //
        Data data=new Data();
        data.set_rssi(1);
        DataMaster m=new DataMaster();
        m.set_rssi(2);
        DataMaster m1=new DataMaster();
        m1.set_rssi(3);
        Log.i("itc",""+data.get_rssi());
        Log.i("itc",""+m.get_rssi());
        Log.i("itc",""+m1.get_rssi());
        //
        Include.init(context.getApplicationContext());
        DataMaster m1=new DataMaster();
        m1.set_rssi(1);
        m1.set_status(Include.STATUS_CONNECT);
        Include.add_master(m1);
        DataMaster m2=new DataMaster();
        m2.set_rssi(2);
        m2.set_status(Include.STATUS_CONNECT);
        Include.add_master(m2);
        for (DataMaster i : Include.get_master())
            Log.i("itc","a:"+i.get_rssi());
        Include.del_master();
        for (DataMaster i : Include.get_master())
            Log.i("itc","b:"+i.get_rssi());
        Include.get_master().get(0).set_status(Include.STATUS_DISCONNECT);
        Include.del_master();
        for (DataMaster i : Include.get_master())
            Log.i("itc","c:"+i.get_rssi());
        Include.emp_master();
        for (DataMaster i : Include.get_master())
            Log.i("itc","d:"+i.get_rssi());*/
        //view
        setContentView(R.layout.activity_main);
        button=findViewById(R.id.button_control);/*
            break;
        case Include.VERSION_DEMO:
            button_outside=findViewById(R.id.button_outside);
            button_query=findViewById(R.id.button_query);
            button_m2m=findViewById(R.id.button_m2m);
            button_adv=findViewById(R.id.button_adv);
            break;
        case Include.VERSION_MANUFACTURE:
            break;
        }*/
        text_view=findViewById(R.id.textview_print);
        swipe_refresh_layout=findViewById(R.id.swiperefreshlayout_print);
        recycler_view=findViewById(R.id.recyclerview_print);
        recycler_view.setVerticalScrollBarEnabled(false);//scrollbar invisible
        layout_manager=new LinearLayoutManager(this);
        recycler_view.setLayoutManager(layout_manager);//bonding RecyclerView & LayoutManager(for ViewHolder)
        layout_manager.setOrientation(OrientationHelper.VERTICAL);
        //data
        adapter_ble=new AdapterBle(this,Include.get_master());//bonding AdapterBle & List<DataBle>
        recycler_view.setAdapter(adapter_ble);//bonding RecyclerView & AdapterBle
        Include.init(context.getApplicationContext());
        //application
        intent=new Intent(this,ServiceBle.class);
        startService(intent);
    }
    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        stopService(intent);
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permission, @NonNull int[] grantResults)
    {
        //dynamic permissions in this function, static permissions in AndroidManifest
        switch(requestCode)
        {
        case Include.REQUEST_CODE_GPS:
            if (!(grantResults.length>0 && grantResults[0]==PackageManager.PERMISSION_GRANTED))
                finish();
            break;
        default:
            super.onRequestPermissionsResult(requestCode,permission,grantResults);
        }
    }
    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
        //
        switch(requestCode)
        {
        case Include.REQUEST_CODE_BLE:
            if (resultCode==RESULT_OK)
            {
                if (!Include.ble_slave())//not support ble slave
                {
                    Toast.makeText(this, "Device not support ble slave", Toast.LENGTH_SHORT).show();
                    finish();
                    //System.exit(0);
                    //ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
                    //manager.killBackgroundProcesses(getPackageName());
                    //manager.restartPackage(getPackageName());
                }
                else
                    service_ble.init();
            }
            else
                finish();
            break;
        case Include.REQUEST_CODE_GPS:
            if (resultCode==0)//gps opened
            {
                if (!Include.gps_app(context) && !ActivityCompat.shouldShowRequestPermissionRationale(ActivityMain.this, Manifest.permission.READ_CONTACTS))//app gps disable && permission request ok
                    ActivityCompat.requestPermissions(ActivityMain.this, new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, Include.REQUEST_CODE_GPS);//app gps setting
            }
            else
                finish();
            break;
        }
    }
    @Override
    public void permission()
    {
        boolean flag;

        flag=true;
        //ble
        if (!Include.ble_support(context))//not support ble
        {
            Toast.makeText(this, "Device not support ble", Toast.LENGTH_SHORT).show();
            flag=false;
            finish();
        }
        if (!Include.ble_enable())//ble disable
        {
            flag = false;
            startActivityForResult(new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE), Include.REQUEST_CODE_BLE);//ble setting
        }
        else if (!Include.ble_slave())//not support ble slave
        {
            Toast.makeText(this, "Device not support ble slave", Toast.LENGTH_SHORT).show();
            flag=false;
            finish();
        }
        //gps
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)//need to enable gps if API>23
        {
            if (!Include.gps_global(context))//global gps disable
            {
                flag = false;
                startActivityForResult(new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS), Include.REQUEST_CODE_GPS);//global gps setting
            }
            else//global gps enable
            {
                if (!Include.gps_app(context) && !ActivityCompat.shouldShowRequestPermissionRationale(ActivityMain.this, Manifest.permission.READ_CONTACTS))//app gps disable && permission request ok
                {
                    flag = false;
                    ActivityCompat.requestPermissions(ActivityMain.this, new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, Include.REQUEST_CODE_GPS);//app gps setting
                }
            }
        }
        if (flag)
            service_ble.init();
    }
    @Override
    public void listen()
    {
        button.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                Log.i("itc","click start");
                service_ble.initial();
            }
        });
        /*
            break;
        case Include.VERSION_DEMO:
            button_outside.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View view)
                {
                    Log.i("itc","click outside");
                    service_ble.preprocess(Include.EVENT_OUTSIDE);
                }
            });
            button_query.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View view)
                {
                    Log.i("itc","click query");
                    service_ble.preprocess(Include.EVENT_QUERY);
                }
            });
            button_m2m.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View view)
                {
                    Log.i("itc","click m2m");
                    service_ble.preprocess(Include.EVENT_M2M);
                }
            });
            button_adv.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View view)
                {
                    Log.i("itc","click adv");
                    service_ble.preprocess(Include.EVENT_ADV);
                }
            });
            break;
        case Include.VERSION_MANUFACTURE:
            break;
        }*/
        service_ble.set_interface(new Interface()
        {
            //initial
            @Override
            public void initial()
            {
                String str;

                str="";/*
                switch(Include.VERSION)
                {
                case Include.VERSION_MODULE:
                    break;
                case Include.VERSION_DEMO:
                    switch(service_ble.get_ret())
                    {
                    case Include.RET_QUERY:
                        break;
                    case Include.RET_M2M:
                        str="(M2M Response)";
                        break;
                    case Include.RET_FIRE:
                        str="(Fire Alarm)";
                        break;
                    }
                    break;
                case Include.VERSION_MANUFACTURE:
                    break;
                }*/
                update_text("Initial"+str);
            }
            //scan
            @Override
            public void scan_init(DataMaster master)
            {
                BluetoothDevice device;

                device=master.get_device();
                if (device!=null)
                    update_text("Central Discovering "+device.getAddress());
            }
            @Override
            public void scan_stop()
            {
                update_text("Central Device "+Include.get_master().size());
                update_list();
            }
            @Override
            public void scan_error(int error)
            {
                update_text("Central Failed "+error);
            }
            //connect
            @Override
            public void connect_init(String address)
            {
                update_text("Central Connect Init");
            }
            @Override
            public void connect_success(String address)
            {
                update_text("Central Connect "+address);
                update_status(Include.STATUS_CONNECT,address);
                update_list();
            }
            @Override
            public void connect_error(String address,int error)
            {
                update_text("Central Failed "+address+" "+error);
            }
            //status
            @Override
            public void status_connect(String address)
            {
                update_status(Include.STATUS_CONNECT,address);
                update_list();
            }
            @Override
            public void status_connecting(String address)
            {
                update_status(Include.STATUS_CONNECTING,address);
                update_list();
            }
            @Override
            public void status_disconnecting(String address)
            {
                update_status(Include.STATUS_DISCONNECTING,address);
                update_list();
            }
            @Override
            public void status_disconnect(String address)
            {
                update_status(Include.STATUS_DISCONNECT,address);/*
                switch(Include.VERSION)
                {
                case Include.VERSION_MODULE:
                    break;
                case Include.VERSION_DEMO:
                    switch(service_ble.get_event())
                    {
                    case Include.EVENT_NONE:
                        //update_condition(service_ble.get_condition(),address);
                        break;
                    case Include.EVENT_OUTSIDE:
                        break;
                    case Include.EVENT_QUERY:
                        break;
                    case Include.EVENT_M2M:
                        break;
                    }
                    break;
                case Include.VERSION_MANUFACTURE:
                    break;
                }*/
                update_list();
            }
            //service
            @Override
            public void service_success(String address)
            {
                update_text("Service Success "+address);
            }
            //characteristic
            @Override
            public void characteristic_r_s(String address)
            {
                update_text("Central Char Read Rsp");
            }
            @Override
            public void characteristic_r_e(String address,int error)
            {
                update_text("Central Char Read Error "+error);
            }
            @Override
            public void characteristic_w_s(String address,byte[] data)
            {
                update_text("Central Char Write Sent "+(char)data[0]);
            }
            @Override
            public void characteristic_w_e(String address,byte[] data,int error)
            {
                update_text("Central Char Write Error "+(char)data[0]+" "+error);
            }
            @Override
            public void characteristic_change(String address,boolean data)
            {
                update_text("Central Char Change");
                update_condition(data,address);
                update_list();
            }
            //descriptor
            @Override
            public void descriptor_r_s(String address)
            {
                update_text("Central Desc Read Rsp");
            }
            @Override
            public void descriptor_r_e(String address,int error)
            {
                update_text("Central Desc Read Error "+error);
            }
            @Override
            public void descriptor_w_s(String address,byte[] data)
            {
                update_text("Central Desc Write Rsp "+data);
            }
            @Override
            public void descriptor_w_e(String address,byte[] data,int error)
            {
                update_text("Central Desc Write Error "+data+" "+error);
            }
            //advertise
            @Override
            public void advertise_init()
            {
                update_text("Peripheral Adv Init");
                update_list();
            }
            @Override
            public void advertise_success()
            {
                update_text("Peripheral Adv Success");
                update_list();
            }
            @Override
            public void advertise_stop()
            {
                update_text("Peripheral Adv Stop");
            }
            @Override
            public void advertise_error(int error)
            {
                update_text("Peripheral Error "+error);
            }
            //status
            @Override
            public void status_connect()
            {
                update_list();
            }
            @Override
            public void status_connecting()
            {

            }
            @Override
            public void status_disconnecting()
            {

            }
            @Override
            public void status_disconnect()
            {
                update_list();
            }
            //characteristic
            @Override
            public void characteristic_r_s()
            {
                //update_text("Peripheral Char Read Rsp");
            }
            @Override
            public void characteristic_r_e(int error)
            {
                update_text("Peripheral Char Read Error "+error);
            }
            @Override
            public void characteristic_w_s(final  byte[] data)
            {
                update_text("Peripheral Char Write Sent "+(char)data[0]);
            }
            @Override
            public void characteristic_w_e(byte[] data,int error)
            {
                update_text("Peripheral Char Write Error "+(char)data[0]+" "+error);
            }
            //descriptor
            @Override
            public void descriptor_r_s()
            {
                update_text("Peripheral Desc Read Rsp");
            }
            @Override
            public void descriptor_r_e(int error)
            {
                update_text("Peripheral Desc Read Error "+error);
            }
            @Override
            public void descriptor_w_s(byte[] data)
            {
                update_text("Peripheral Desc Write Sent "+data);
            }
            @Override
            public void descriptor_w_e(byte[] data,int error)
            {
                update_text("Peripheral Desc Write Error "+data+" "+error);
            }
        });
    }
    //update status(master)
    private void update_status(byte status,String address)
    {
        BluetoothDevice device;

        for (DataMaster i : Include.get_master())
        {
            device = i.get_device();
            if (device != null && device.getAddress().equals(address))
            {
                i.set_status(status);
                break;
            }
        }
    }
    //update condition(master)
    private void update_condition(boolean condition,String address)
    {
        BluetoothDevice device;

        for (DataMaster i : Include.get_master())
        {
            device = i.get_device();
            if (device != null && device.getAddress().equals(address))
            {
                i.set_condition(condition);
                break;
            }
        }
    }/*
    //update status(slave)
    private void update_status(byte status)
    {
        Include.get_master().get(0).set_status(status);
    }*/
    //update text
    public void update_text(String string)
    {
        text_view.setText(string);
    }
    //update list
    public void update_list()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                adapter_ble.update(Include.get_master());
            }
        });
    }
}