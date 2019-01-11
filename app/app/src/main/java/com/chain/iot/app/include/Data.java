package com.chain.iot.app.include;

import java.io.Serializable;

public class Data implements Serializable
{
    private byte status;//device status
    private byte[] buffer;//send/recv data
    //
    public Data()
    {
        super();//for implements, not for extends
        switch(Include.DEBUG)
        {
        case Include.DEBUG_BOTH:
            buffer=new byte[2];
            break;
        default:
            buffer=new byte[1];
            break;
        }
    }
    public void set_status(byte status)
    {
        this.status=status;
    }
    public byte get_status()
    {
        return status;
    }
    public void set_buffer(byte[] buffer)
    {
        this.buffer=buffer;
    }
    public byte[] get_buffer()
    {
        return buffer;
    }
}