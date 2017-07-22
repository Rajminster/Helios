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
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
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


        mLEScanner = mBluetoothAdapter.getBluetoothLeScanner();
        settings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .build();
        filters = new ArrayList<ScanFilter>();
        scanLeDevice(true);
        if (device != null)
            connectToDevice(device);
        else {
            device = bleDevice.getBluetoothDevice();
            connectToDevice(device);
        }
    }

    private ScanCallback mLeScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            Log.d("PRINT", result.toString());
        }
    };

    public void connectToDevice(BluetoothDevice device) {
        if (mGatt == null) {
            mGatt = device.connectGatt(this, false, gattCallback);
//            BluetoothGattService service = mGatt.getService(serviceUUID);
//            List<BluetoothGattCharacteristic> list = service.getCharacteristics();
//            BluetoothGattCharacteristic cha = list.get(0);
//            Log.d("FUUCKCKKCKKKC", cha.toString());
            scanLeDevice(false);// will stop after first device detection
        }
    }

    private void scanLeDevice(final boolean enable) {
        if (enable) {
            if (Build.VERSION.SDK_INT < 21)
//                        mBluetoothAdapter.stopLeScan(mLeScanCallback);
                mBluetoothAdapter.getBluetoothLeScanner().startScan(mLeScanCallback);
        }
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.i("onConnectionStateChange", "Status: " + status);
            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    Log.i("gattCallback", "STATE_CONNECTED");
                    gatt.discoverServices();
                    Log.d("FUUCKCKKCKKKC", gatt.getDevice().getName());
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
            Log.i("onServicesDiscovered", services.toString());
            gatt.readCharacteristic(services.get(2).getCharacteristics().get(0));
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic
                                                 characteristic, int status) {
            Log.i("onCharacteristicRead", characteristic.getUuid().toString());
            Log.i("onCharacteristicValue", characteristic.getValue().toString());
            gatt.readDescriptor(characteristic.getDescriptors().get(0));
            //gatt.disconnect();
        }
        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                                     int status) {
            Log.i("onDescriptorRead", descriptor.getUuid().toString());
            Log.i("onDescriptorValue", descriptor.getValue().toString());
        }
    };
}



