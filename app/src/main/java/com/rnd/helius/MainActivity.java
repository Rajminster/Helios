package com.rnd.helius;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
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
    ConnectedThread mConnectedThread;
    String uuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    ArrayList<HashMap<String, String>> gattServiceData =
            new ArrayList<HashMap<String, String>>();
    ArrayList<ArrayList<HashMap<String, String>>> gattCharacteristicData
            = new ArrayList<ArrayList<HashMap<String, String>>>();
    ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics =
            new ArrayList<ArrayList<BluetoothGattCharacteristic>>();

    final int handlerState = 0;                         //used to identify handler message
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private StringBuilder recDataString = new StringBuilder();
    BluetoothDevice tracker;

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

        BluetoothAdapter bluetooth = BluetoothAdapter.getDefaultAdapter();
        if (bluetooth != null && !bluetooth.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
        }
        Set<BluetoothDevice> pairedDevices = bluetooth.getBondedDevices();
        ArrayList list = new ArrayList();
        for (BluetoothDevice bt : pairedDevices) {
            if (bt.getName().equals("Solar Tracker")) {
                tracker = bt;
                Log.d("FOUND", "FOUND");
            }
            list.add(bt.getName());
        }

        BluetoothManager bluetoothManager = (BluetoothManager) getApplicationContext().getSystemService(Context.BLUETOOTH_SERVICE);
        List<BluetoothDevice> devices = bluetoothManager.getConnectedDevices(BluetoothProfile.GATT);
        for(BluetoothDevice device : devices) {
            if(device.getType() == BluetoothDevice.DEVICE_TYPE_LE) {
                Log.d("GOT", "EM");
            }
        }
        BluetoothGattService gattService = new BluetoothGattService(UUID.fromString(uuid), BluetoothGattService.SERVICE_TYPE_PRIMARY);
        BluetoothGattCharacteristic cha = gattService.getCharacteristic(UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"));
        //List<BluetoothGattDescriptor> des =  cha.getDescriptors();
       // Log.d("CHA CHA", cha.toString());

    }

    @Override
    public void onPause() {
        super.onPause();
        if (tracker != null) {
            try {
                //Don't leave Bluetooth sockets open when leaving activity
                btSocket.close();
            } catch (IOException e2) {
                //insert code to deal with this
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (tracker != null) {
            //Get MAC address from DeviceListActivity via intent
            Intent intent = getIntent();

            //Get the MAC address from the DeviceListActivty via EXTRA
            address = tracker.getAddress();

            //create device and set the MAC address
            //BluetoothDevice device = btAdapter.getRemoteDevice(address);

            try {
                btSocket = tracker.createRfcommSocketToServiceRecord(UUID.fromString(uuid));
            } catch (IOException e) {
                Toast.makeText(getBaseContext(), "Socket creation failed", Toast.LENGTH_LONG).show();
            }
            // Establish the Bluetooth socket connection.
            try {
                btSocket.connect();
            } catch (IOException e) {
                try {
                    btSocket.close();
                } catch (IOException e2) {
                    //insert code to deal with this
                }
            }
            mConnectedThread = new ConnectedThread(btSocket);
            mConnectedThread.start();
            mConnectedThread.run();

            //I send a character when resuming.beginning transmission to check device is connected
            //If it is not an exception will be thrown in the write method and finish() will be called
            //mConnectedThread.write("x");
        }
    }

    //create new class for connect thread
    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        //creation of the connect thread
        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                //Create I/O streams for connection
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }


        public void run() {
            byte[] buffer = new byte[256];
            int bytes;

            // Keep looping to listen for received messages
            while (true) {
                try {
                    bytes = mmInStream.read(buffer);            //read bytes from input buffer
                    String readMessage = new String(buffer, 0, bytes);
                    // Send the obtained bytes to the UI Activity via handler
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                    Log.d("LOOP", readMessage);
                } catch (IOException e) {
                    break;
                }
            }
        }

        //write method
        public void write(String input) {
            byte[] msgBuffer = input.getBytes();           //converts entered String into bytes
            try {
                mmOutStream.write(msgBuffer);                //write bytes over BT connection via outstream
            } catch (IOException e) {
                //if you cannot write, close the application
                Toast.makeText(getBaseContext(), "Connection Failure", Toast.LENGTH_LONG).show();
                finish();

            }
        }
    }
}
