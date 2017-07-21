package com.rnd.helius;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
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
import com.polidea.rxandroidble.RxBleConnection;
import com.polidea.rxandroidble.RxBleDevice;
import com.polidea.rxandroidble.internal.RxBleLog;

import java.util.Set;
import java.util.UUID;

import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;

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
    UUID uldr = UUID.fromString(ldr);

    BluetoothDevice tracker;
    BluetoothAdapter bluetooth;
    RxBleDevice bleDevice;

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

        bluetooth = BluetoothAdapter.getDefaultAdapter();
        if (bluetooth != null && !bluetooth.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
        }
        Set<BluetoothDevice> pairedDevices = bluetooth.getBondedDevices();
        for (BluetoothDevice bt : pairedDevices) {
            if (bt.getName().equals("Helius Panel")) {
                tracker = bt;
//                Log.d("FOUND", bt.getName());
//                BluetoothSocket sock;
//                try {
//                    sock = tracker.createInsecureRfcommSocketToServiceRecord(UUID.fromString(ldr));
//                    sock.connect();
//                    InputStreamReader is = new InputStreamReader(sock.getInputStream());
//                    BufferedReader br = new BufferedReader(is);
//                    String line;
//                    while ((line = br.readLine()) != null) {
//                        Log.d("READ", line);
//                    }
//                    br.close();
//                } catch (IOException e) {
//                    e.printStackTrace();
//                }
            }
        }
        /**
         * Bluetooth Low Energy client code using the RxAndroidBle library
         * Library can be found here: https://github.com/Polidea/RxAndroidBle
         */
        RxBleClient rxBleClient = RxBleClient.create(this);
        RxBleClient.setLogLevel(RxBleLog.DEBUG);

//
//        /**
//         * Scan for Bluetooth Low Energy Devices
//         */
//        Subscription scanSubscription = rxBleClient.scanBleDevices(
//                new ScanSettings.Builder()
//                        // .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY) // change if needed
//                        // .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES) // change if needed
//                        .build()
//                // add filters if needed
//        )
//                .subscribe(
//                        scanResult -> {
//                            // Process scan result here.
//                            Log.d("BLE: ", scanResult.toString());
//                        },
//                        throwable -> {
//                            // Handle an error here.
//                        }s
//                );

// When done, just unsubscribe.
//        scanSubscription.unsubscribe();

        String macAddress = "98:4F:EE:10:AB:C3";
        bleDevice = rxBleClient.getBleDevice(macAddress);

        Subscription subscription = bleDevice.establishConnection(this, true)
                .flatMap(rxBleConnection -> rxBleConnection.readCharacteristic(uldr))
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(bytes -> {
                    // Read characteristic value.
                    Log.d("TEST", bytes.toString());
                });

// When done... unsubscribe and forget about connection teardown :)
        subscription.unsubscribe();

//        Subscription subscription2 = bleDevice.establishConnection(true)
//                .flatMap(rxBleConnection -> rxBleConnection.readCharacteristic(UUID.fromString(ldr)))
//                .subscribe(
//                        characteristicValue -> {
//                            // Read characteristic value.
//                            Toast.makeText(this, characteristicValue.toString(), Toast.LENGTH_SHORT).show();
//                            Log.d("Fuck", characteristicValue.toString());
//                        },
//                        throwable -> {
//                            // Handle an error here.
//                        }
//                );
//        subscription2.unsubscribe();
//        PublishSubject<Void> disconnectTriggerSubject = PublishSubject.create();
//        Observable<RxBleConnection> connectionObservable = device
//                .establishConnection(false)
//                .takeUntil(disconnectTriggerSubject)
//                .compose(bindUntilEvent(PAUSE))
//                .compose(new ConnectionSharingAdapter());
//
//        connectionObservable
//                .flatMap(rxBleConnection -> rxBleConnection.readCharacteristic(UUID.fromString(ldr)))
//                .observeOn(AndroidSchedulers.mainThread())
//                .subscribe(bytes -> {
//                    readOutputView.setText(new String(bytes));
//                    readHexOutputView.setText(HexString.bytesToHex(bytes));
//                    writeInput.setText(HexString.bytesToHex(bytes));
//                }, this::onReadFailure);

    }

    private boolean isConnected() {
        return bleDevice.getConnectionState() == RxBleConnection.RxBleConnectionState.CONNECTED;
    }

}
