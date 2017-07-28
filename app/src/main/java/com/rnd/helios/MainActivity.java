package com.rnd.helios;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
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

/**
 * Activity to setup Bluetooth connectivity and update BLE characteristics on the UI based on the
 * readings from the sensors corresponding to each characteristic. This activity will handle all the
 * requirements and use of Bluetooth Low Energy in order to connect, receive, and trasmit data.
 *
 * @author Rishi Raj
 */
public class MainActivity extends AppCompatActivity {
    Button searchButton;
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
    BluetoothDevice device;
    BluetoothAdapter mBluetoothAdapter;
    RxBleDevice bleDevice;
    BluetoothGatt mGatt;
    BluetoothGattService helios;
    BluetoothGatt bluetoothGatt;
    List<BluetoothGattCharacteristic> heliosChars;
    boolean power = false;
    boolean search = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        /* Initialize UI elements */
        ldr0Button = (Button) findViewById(R.id.ldr0);
        ldr0Button.setText(0);
        ldr0Button.setEnabled(false);
        ldr1Button = (Button) findViewById(R.id.ldr1);
        ldr1Button.setText(0);
        ldr1Button.setEnabled(false);
        ldr2Button = (Button) findViewById(R.id.ldr2);
        ldr2Button.setText(0);
        ldr2Button.setEnabled(false);
        ldr3Button = (Button) findViewById(R.id.ldr3);
        ldr3Button.setText(0);
        ldr3Button.setEnabled(false);
        powerBar = (ProgressBar) findViewById(R.id.powerBar);
        powerBar.setProgress(0);
        powerText = (TextView) findViewById(R.id.powerText);
        powerButton = (Button) findViewById(R.id.powerButton);
        powerButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                power = true;
                if (mGatt != null) {
                    Log.i("POWER", "Set Value " + power);
                }
            }
        });
        searchButton = (Button) findViewById(R.id.searchButton);
        searchButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                search = true;
                if (mGatt != null) {
                    Log.i("Search", "Search " + power);
                }
            }
        });
        findButton = (Button) findViewById(R.id.findButton);
        findButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

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
            public void handleMessage(android.os.Message msg) {
                String readMessage = (String) msg.obj;
                Toast.makeText(getApplicationContext(), readMessage, Toast.LENGTH_SHORT).show();
                Log.d("FOUND", readMessage);
            }
        };

        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        if (mBluetoothAdapter == null && !mBluetoothAdapter.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
        }
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
        for (BluetoothDevice bt : pairedDevices) {
            if (bt.getName().equals("Helios Panel")) {
                device = bt;
                Log.d("FOUND", bt.getName());

            }
        }
        RxBleClient rxBleClient = RxBleClient.create(this);
        RxBleClient.setLogLevel(RxBleLog.DEBUG);
        String macAddress = "98:4F:EE:10:AB:C3";
        bleDevice = rxBleClient.getBleDevice(macAddress);

        if (device != null)
            connectToDevice(device);
        else {
            device = bleDevice.getBluetoothDevice();
            connectToDevice(device);
        }

        final Handler handler = new Handler();
        Runnable task = new Runnable() {
            @Override
            public void run() {
                Log.d("LOOP", "Doing task");
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
            helios = services.get(2);
            heliosChars = helios.getCharacteristics();
            chars = helios.getCharacteristics();
            Log.i("onServicesDiscovered", heliosChars.get(5).getUuid().toString());
            for (BluetoothGattCharacteristic c : heliosChars) {
                gatt.setCharacteristicNotification(c, true);
            }
            requestCharacteristics(gatt);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    findButton.setVisibility(View.GONE);
                    ldr0Button.setEnabled(true);
                    ldr1Button.setEnabled(true);
                    ldr2Button.setEnabled(true);
                    ldr3Button.setEnabled(true);
                }
            });
        }

        public void requestCharacteristics(BluetoothGatt gatt) {
            if (chars.isEmpty()) {
                chars.addAll(heliosChars);
                requestCharacteristics(gatt);
            } else {
                gatt.readCharacteristic(chars.get(0));
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (characteristic.getUuid().toString().charAt(7) == '2')
                        ldr0Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '3')
                        ldr1Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '4')
                        ldr2Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '5')
                        ldr3Button.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    if (characteristic.getUuid().toString().charAt(7) == '6') {
                        powerText.setText(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + " Wh");
                        powerBar.setProgress(characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0));
                    }
                    if (characteristic.getUuid().toString().charAt(7) == '7') {
                        if (power) {
                            characteristic.setValue(1, BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                            gatt.writeCharacteristic(characteristic);
                            power = false;
                        }
                        Log.i("Power", characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    }
                    if (characteristic.getUuid().toString().charAt(7) == '8') {
                        if (search) {
                            characteristic.setValue(1, BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                            gatt.writeCharacteristic(characteristic);
                            search = false;
                        }
                        Log.i("Search", characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT8, 0) + "");
                    }
                }
            });
            chars.add(chars.remove(0));
            if (chars.size() > 0) {
                requestCharacteristics(gatt);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic, int status) {
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
