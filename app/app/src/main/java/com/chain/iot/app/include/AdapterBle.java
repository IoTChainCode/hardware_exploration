package com.chain.iot.app.include;

import android.support.v7.widget.RecyclerView;
import android.content.Context;
import java.util.List;

import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import com.chain.iot.app.R;
import com.chain.iot.app.activity.ActivityBase;
import com.chain.iot.app.service.ServiceBle;

import android.view.LayoutInflater;
import android.bluetooth.BluetoothDevice;
import android.widget.TextView;

public class AdapterBle extends RecyclerView.Adapter<AdapterBle.ViewHolder>
{
    private Context context;
    private List<DataMaster> master;
    //
    public AdapterBle(Context context,List<DataMaster> master)
    {
        this.context=context;
        this.master=master;
    }
    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType)
    {
        View view = LayoutInflater.from(context).inflate(R.layout.recyclerview_ble,parent,false);
        return new ViewHolder(view);
    }
    @Override
    public void onBindViewHolder(ViewHolder holder, final int position)
    {
        final DataMaster master;

        master=this.master.get(position);
        if (master!=null)
        {
            BluetoothDevice device=master.get_device();
            //if (Include.get_role()==Include.ROLE_CENTRAL)
            //{
                if (device!=null)
                {
                    holder.text_name.setText("Name:" + device.getName());
                    holder.text_address.setText("Address:" + device.getAddress());
                }
                else
                {
                    holder.text_name.setText("Name:");
                    holder.text_address.setText("Address:");
                }
                holder.text_status.setText("Status:" + get_status(master.get_status()));
                holder.text_condition.setText("Condition:" + get_condition(master.get_condition()));
                holder.text_rssi.setText("RSSI:" + master.get_rssi());
            //}
            //
            holder.itemView.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View view)
                {
                    ActivityBase activity_base;
                    byte[] buffer=new byte[2];

                    Log.i("itc","adapter click "+position);
                    activity_base=(ActivityBase)context;
                    //activity_base.service_ble.set_event(Include.EVENT_NONE);
                    //activity_base.service_ble.connect(master.get_device().getAddress());
                    buffer[0]=Include.CMD_CLOCK;
                    if (master.get_condition())
                    {
                        buffer[1]=Include.FALSE;
                        master.set_condition(Include.SWITCH_OFF);
                    }
                    else
                    {
                        buffer[1]=Include.TRUE;
                        master.set_condition(Include.SWITCH_ON);
                    }
                    activity_base.service_ble.transfer(master.get_gatt(),master.get_characteristic(),buffer);
                }
            });
        }
    }
    @Override
    public int getItemCount()
    {
        return master==null ? 0 : master.size();
    }
    public static class ViewHolder extends RecyclerView.ViewHolder
    {
        private TextView text_name;
        private TextView text_status;
        private TextView text_address;
        private TextView text_condition;
        private TextView text_rssi;
        //
        public ViewHolder(View itemView)
        {
            super(itemView);
            text_name=itemView.findViewById(R.id.ble_name);
            text_status=itemView.findViewById(R.id.ble_status);
            text_address=itemView.findViewById(R.id.ble_address);
            text_condition=itemView.findViewById(R.id.ble_condition);
            text_rssi=itemView.findViewById(R.id.ble_rssi);
        }
    }
    private String get_status(byte status)
    {
        switch(status)
        {
        case Include.STATUS_SCANNING:
            return "Scanning";
        case Include.STATUS_CONNECT:
            return "Connect";
        case Include.STATUS_CONNECTING:
            return "Connecting";
        case Include.STATUS_DISCONNECTING:
            return "Disconnecting";
        case Include.STATUS_DISCONNECT:
            return "Disconnect";
        case Include.STATUS_SERVICES:
            return "Services";
        case Include.STATUS_ADVERTISING:
            return "Advertising";
        default:
            return "";
        }
    }
    private String get_condition(boolean condition)
    {
        return condition==Include.SWITCH_OFF ? "OFF" : "ON";
    }
    //update adapter(after data download finished)
    public void update(List<DataMaster> master)
    {
        this.master=master;
        notifyDataSetChanged();
    }
}