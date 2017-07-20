package com.rnd.helius;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    Button findButton;
    Button ldr0Button;
    Button ldr1Button;
    Button ldr2Button;
    Button ldr3Button;
    Button powerButton;
    Button helpButton;
    ProgressBar powerBar;
    TextView powerText;
    Handler bluetoothIn;
    static String address;
    String uuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    String ldr = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

    BluetoothDevice tracker;
    BluetoothAdapter bluetooth;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ldr0Button = (Button) findViewById(R.id.ldr0);
        ldr0Button.setText((int) (Math.random() * 100) + "");
        ldr1Button = (Button) findViewById(R.id.ldr1);
        ldr1Button.setText((int) (Math.random() * 100) + "");
        ldr2Button = (Button) findViewById(R.id.ldr2);
        ldr2Button.setText((int) (Math.random() * 100) + "");
        ldr3Button = (Button) findViewById(R.id.ldr3);
        ldr3Button.setText((int) (Math.random() * 100) + "");

        powerBar = (ProgressBar) findViewById(R.id.powerBar);
        powerBar.setProgress((int) (Math.random() * 100));

        powerText = (TextView) findViewById(R.id.powerText);

        powerButton = (Button) findViewById(R.id.powerButton);
        powerButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

            }
        });

        findButton = (Button) findViewById(R.id.findButton);
        findButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(getApplicationContext(), BluetoothActivity.class);
                startActivity(intent);
            }
        });

        helpButton = (Button) findViewById(R.id.helpButton);
        helpButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(getApplicationContext(), HelpActivity.class);
                startActivity(intent);
            }
        });

        bluetoothIn = new Handler() {
            public void handleMessage(android.os.Message msg) {                 //if message is what we want
                String readMessage = (String) msg.obj;                                                                // msg.arg1 = bytes from connect thread
                Toast.makeText(getApplicationContext(), readMessage, Toast.LENGTH_SHORT).show();
                Log.d("FOUND", readMessage);
            }
        };

        bluetooth = BluetoothAdapter.getDefaultAdapter();
        if (bluetooth != null && !bluetooth.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
        }
        Set<BluetoothDevice> pairedDevices = bluetooth.getBondedDevices();
        for (BluetoothDevice bt : pairedDevices) {
            if (bt.getName().equals("Helius Panel")) {
                tracker = bt;
                Log.d("FOUND", bt.getName());
            }
        }
    }

    private static final String TAG = "BLEDevice";

    public static final String EXTRA_BLUETOOTH_DEVICE = "BT_DEVICE";
    private BluetoothAdapter mBTAdapter;
    private BluetoothDevice mDevice;
    private BluetoothGatt mConnGatt;
    private int mStatus;


    private final BluetoothGattCallback mGattcallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status,
                                            int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                mStatus = newState;
                mConnGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                mStatus = newState;
                runOnUiThread(new Runnable() {
                    public void run() {

                    }

                    ;
                });
            }
        }

        ;

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            for (BluetoothGattService service : gatt.getServices()) {
                if ((service == null) || (service.getUuid() == null)) {
                    continue;
                }
                runOnUiThread(new Runnable() {
                    public void run() {

                    }

                    ;
                });
            }

        }

        ;

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (ldr.equalsIgnoreCase(characteristic.getUuid().toString())) {
                    final String name = characteristic.getStringValue(0);

                    runOnUiThread(new Runnable() {
                        public void run() {
//                            mReadManufacturerNameButton.setText(name);
                            Log.d("YAAAAAAAAAAAAAAAAAAAAY?", "I HAT ETHIS FUCKING SHT");

                        }

                        ;
                    });
                }
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                                          BluetoothGattCharacteristic characteristic, int status) {
        }
    };


    @Override
    protected void onResume() {
        super.onResume();

        init();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mConnGatt != null) {
            if ((mStatus != BluetoothProfile.STATE_DISCONNECTING)
                    && (mStatus != BluetoothProfile.STATE_DISCONNECTED)) {
                mConnGatt.disconnect();
            }
            mConnGatt.close();
            mConnGatt = null;
        }
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.read_manufacturer_name_button) {
            if ((v.getTag() != null)
                    && (v.getTag() instanceof BluetoothGattCharacteristic)) {
                BluetoothGattCharacteristic ch = (BluetoothGattCharacteristic) v
                        .getTag();
                if (mConnGatt.readCharacteristic(ch)) {
                    setProgressBarIndeterminateVisibility(true);
                }
            }
        } else if (v.getId() == R.id.read_serial_number_button) {
            if ((v.getTag() != null)
                    && (v.getTag() instanceof BluetoothGattCharacteristic)) {
                BluetoothGattCharacteristic ch = (BluetoothGattCharacteristic) v
                        .getTag();
                if (mConnGatt.readCharacteristic(ch)) {
                    setProgressBarIndeterminateVisibility(true);
                }
            }

        } else if (v.getId() == R.id.write_alert_level_button) {
            if ((v.getTag() != null)
                    && (v.getTag() instanceof BluetoothGattCharacteristic)) {
                BluetoothGattCharacteristic ch = (BluetoothGattCharacteristic) v
                        .getTag();
                ch.setValue(new byte[]{(byte) 0x03});
                if (mConnGatt.writeCharacteristic(ch)) {
                    setProgressBarIndeterminateVisibility(true);
                }
            }
        }
    }

    private void init() {
        mBTAdapter = bluetooth;

        if (mBTAdapter == null) {
            Toast.makeText(this, "GET FUCKED", Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }

        // check BluetoothDevice
        if (mDevice == null) {
            mDevice = getBTDeviceExtra();
            if (mDevice == null) {
                finish();
                return;
            }
        }

        // connect to Gatt
        if ((mConnGatt == null)
                && (mStatus == BluetoothProfile.STATE_DISCONNECTED)) {
            // try to connect
            mConnGatt = mDevice.connectGatt(this, false, mGattcallback);
            mStatus = BluetoothProfile.STATE_CONNECTING;
        } else {
            if (mConnGatt != null) {
                // re-connect and re-discover Services
                mConnGatt.connect();
                mConnGatt.discoverServices();
            } else {
                Log.e(TAG, "state error");
                finish();
                return;
            }
        }
    }

    private BluetoothDevice getBTDeviceExtra() {
        Intent intent = getIntent();
        if (intent == null) {
            return null;
        }

        Bundle extras = intent.getExtras();
        if (extras == null) {
            return null;
        }

        return extras.getParcelable(EXTRA_BLUETOOTH_DEVICE);
    }

}
