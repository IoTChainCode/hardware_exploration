package com.chain.iot.app.include;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import com.chain.iot.app.activity.ActivityBase;
import com.chain.iot.app.service.ServiceBle;

public class ServiceConnectionInstance implements ServiceConnection
{
    private ActivityBase activity_base;

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder)//execute after service connect
    {
        activity_base.service_ble=((ServiceBle.BinderBle) iBinder).get_service();
        activity_base.permission();
        activity_base.listen();
    }
    @Override
    public void onServiceDisconnected(ComponentName componentName)//execute after service disconnect
    {
    }
    //
    public ServiceConnectionInstance(ActivityBase activity_base)
    {
        this.activity_base=activity_base;
    }
}