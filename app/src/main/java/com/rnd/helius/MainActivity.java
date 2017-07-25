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
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.polidea.rxandroidble.RxBleClient;
import com.polidea.rxandroidble.RxBleDevice;
import com.polidea.rxandroidble.internal.RxBleLog;

import java.util.ArrayList;
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
    String ser = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    String ldr = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    UUID uldr = UUID.fromString(ldr);
    UUID serviceUUID = UUID.fromString(ser);
    BluetoothDevice device;
    BluetoothAdapter mBluetoothAdapter;
    RxBleDevice bleDevice;
    private int REQUEST_ENABLE_BT = 1;
    private Handler mHandler;
    private static final long SCAN_PERIOD = 10000;
    private BluetoothLeScanner mLEScanner;
    private ScanSettings settings;
    private List<ScanFilter> filters;
    private BluetoothGatt mGatt;
    List<BluetoothGattService> services;
    BluetoothGattService helius;
    BluetoothGatt bluetoothGatt;
    List<BluetoothGattCharacteristic> heliusChars;
    boolean power = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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
                power = true;
                if (mGatt != null) {
//                    heliusChars.get(5).setValue(1,BluetoothGattCharacteristic.FORMAT_UINT8,0);
                    Log.i("POWER", "Set Value " + power);

                }
            }
        });
        findButton = (Button) findViewById(R.id.findButton);
        findButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
//                Intent intent = new Intent(getApplicationContext(), BluetoothActivity.class);
//                startActivity(intent)
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
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
//        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null && !mBluetoothAdapter.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
        }
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        for (BluetoothDevice bt : pairedDevices) {
            if (bt.getName().equals("Helius Panel")) {
                device = bt;
                Log.d("FOUND", bt.getName());

            }
        }
        RxBleClient rxBleClient = RxBleClient.create(this);
        RxBleClient.setLogLevel(RxBleLog.DEBUG);
        String macAddress = "98:4F:EE:10:AB:C3";
        bleDevice = rxBleClient.getBleDevice(macAddress);

        settings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .build();
        filters = new ArrayList<ScanFilter>();
        //scanLeDevice(true);
        if (device != null)
            connectToDevice(device);
        else {
            device = bleDevice.getBluetoothDevice();
            connectToDevice(device);
        }

        //Loop
        final Handler handler = new Handler();
        Runnable task = new Runnable() {
            @Override
            public void run() {
                Log.d("LOOP", "Doing task");
                if (heliusChars != null) {

                }
                handler.postDelayed(this, 1000);
            }
        };
        handler.post(task);
    }


    public void connectToDevice(BluetoothDevice device) {
        if (mGatt == null) {
            mGatt = device.connectGatt(this, false, gattCallback);
        }
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        List<BluetoothGattCharacteristic> chars = new ArrayList<>();

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.i("onConnectionStateChange", "Status: " + status);
            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    Log.i("gattCallback", "STATE_CONNECTED");
                    gatt.discoverServices();
                    Log.d("FUUCKCKKCKKKC", gatt.getDevice().getName());
                    bluetoothGatt = gatt;
                    break;
                case BluetoothProfile.STATE_DISCONNECTED:
                    Log.e("gattCallback", "STATE_DISCONNECTED");
                    break;
                default:
                    Log.e("gattCallback", "STATE_OTHER");
            }

        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            List<BluetoothGattService> services = gatt.getServices();
            helius = services.get(2);
            heliusChars = helius.getCharacteristics();
            chars = helius.getCharacteristics();
            Log.i("onServicesDiscovered", heliusChars.get(5).getUuid().toString());
            //gatt.readCharacteristic(heliusChars.get(0));
            for (BluetoothGattCharacteristic c : heliusChars)
                gatt.setCharacteristicNotification(c, true);
            requestCharacteristics(gatt);
        }

        public void requestCharacteristics(BluetoothGatt gatt) {
            if (chars.isEmpty()) {
                chars.addAll(heliusChars);
                requestCharacteristics(gatt);
            } else
                gatt.readCharacteristic(chars.get(0));
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
//            Log.i("onCharacteristicRead", characteristic.getUuid().toString());
//            Log.i("onCharacteristicValue", characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");

            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (characteristic.getUuid().toString().charAt(7) == '2')
//                        ldr0Button.setText(Math.round(Math.random() * 100) + "%");
                        ldr0Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '3')
//                        ldr1Button.setText(Math.round(Math.random() * 100) + "%");
                        ldr1Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '4')
//                        ldr2Button.setText(Math.round(Math.random() * 100) + "%");
                        ldr2Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '5')
//                        ldr3Button.setText(Math.round(Math.random() * 100) + "%");
                        ldr3Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '7') {
//                        powerText.setText(Math.round(Math.random() * 100) + "%");
                        powerText.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8
                                , 0) + "");
                        powerBar.setProgress(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0));
                        if (power) {
                            characteristic.setValue(1, BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                            gatt.writeCharacteristic(characteristic);
                            power = false;
                        }
                        Log.i("Power", characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    }
                }
            });


            // gatt.readDescriptor(characteristic.getDescriptors().get(0));
            //gatt.disconnect();
            chars.add(chars.remove(0));


            if (chars.size() > 0) {
                requestCharacteristics(gatt);
            } else {

            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.i("onCharacteristicWrite", characteristic.getUuid().toString());
        }

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                                     int status) {
            Log.i("onDescriptorRead", descriptor.getUuid().toString());
            Log.i("onDescriptorValue", descriptor.getValue().toString());
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic
                characteristic) {
            Log.i("onCharacteristicChange", characteristic.getUuid().toString() + "CHANGED");
        }
    };
}



