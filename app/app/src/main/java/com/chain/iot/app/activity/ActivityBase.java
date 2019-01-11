package com.chain.iot.app.activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import com.chain.iot.app.include.ServiceConnectionInstance;
import com.chain.iot.app.service.ServiceBle;

public class ActivityBase extends AppCompatActivity
{
    public ServiceBle service_ble;
    protected Context context;
    private ServiceConnectionInstance service_connection_instance;
    private Intent intent;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        //
        context=this;
        service_connection_instance=new ServiceConnectionInstance((ActivityBase)context);
        intent=new Intent(this,ServiceBle.class);
        bindService(intent,service_connection_instance,Context.BIND_AUTO_CREATE);//binding service
    }
    //
    public void permission()//check permission setting
    {
    }
    public void listen()//listen gatt/gattserver callback and click event
    {
    }
}
