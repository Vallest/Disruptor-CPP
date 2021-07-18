package com.disruptor_cpp.tests;

import androidx.appcompat.app.AppCompatActivity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.TextView;
import android.text.method.ScrollingMovementMethod;

public class MainActivity extends AppCompatActivity
{
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        tv_ = new TextView(this);
        tv_.setMovementMethod(new ScrollingMovementMethod());

        setContentView(tv_);

        setAssetManager(getAssets());

        (new Thread() {
            @Override
            public void run() {
                runTestCases();
            }
        }).start();
    }

    public void message(String s)
    {
        Message msg = new Message();

        msg.what = 0;
        msg.obj = s;

        handler_.sendMessage(msg);
    }

    public native void setAssetManager(AssetManager assetManager);

    public native void runTestCases();

    private Handler handler_ = new Handler() {
        @Override
        public void handleMessage(Message msg)
        {
            super.handleMessage(msg);
            switch (msg.what)
            {
                case 0:
                    tv_.append((String)msg.obj);
                    tv_.post(new Runnable() {
                        @Override
                        public void run() {
                            int n = tv_.getLayout().getLineTop(tv_.getLineCount()) - tv_.getHeight();
                            if (n > 0) {
                                tv_.scrollTo(0, n);
                            }
                            else {
                                tv_.scrollTo(0, 0);
                            }
                        }
                    });
                    break;
            }
        }
    };

    private TextView tv_;
}

