package com.chain.iot.app.include;

public interface Interface
{
    //initial
    void initial();//initial
    //scan
    void scan_init(DataMaster master);//scan init
    void scan_stop();//scan stop
    void scan_error(int error);//scan error
    //connect
    void connect_init(String address);//connect init
    void connect_success(String address);//connect success
    void connect_error(String address,int error);//connect error
    //status
    void status_connect(String address);//connect
    void status_connecting(String address);//connecting
    void status_disconnecting(String address);//disconnecting
    void status_disconnect(String address);//disconnect
    //service
    void service_success(String address);//find service
    //characteristic
    void characteristic_r_s(String address);
    void characteristic_r_e(String address,int error);
    void characteristic_w_s(String address,byte[] data);
    void characteristic_w_e(String address,byte[] data,int error);
    void characteristic_change(String address,boolean data);
    //descriptor
    void descriptor_r_s(String address);
    void descriptor_r_e(String address,int error);
    void descriptor_w_s(String address,byte[] data);
    void descriptor_w_e(String address,byte[] data,int error);
    //advertise
    void advertise_init();//adv init
    void advertise_success();//adv success
    void advertise_stop();//adv stop
    void advertise_error(int error);//adv error
	//status
	void status_connect();//connect
    void status_connecting();//connecting
    void status_disconnecting();//disconnecting
    void status_disconnect();//disconnect
    //characteristic
    void characteristic_r_s();
    void characteristic_r_e(int error);
    void characteristic_w_s(byte[] data);
    void characteristic_w_e(byte[] data,int error);
    //descriptor
    void descriptor_r_s();
    void descriptor_r_e(int error);
    void descriptor_w_s(byte[] data);
    void descriptor_w_e(byte[] data,int error);
}