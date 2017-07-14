package com.rnd.helius;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Set;

public class BluetoothActivity extends AppCompatActivity {

    Button scanButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bluetooth);

        scanButton = (Button) findViewById(R.id.scanButton);
        scanButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent discoverableIntent =
                        new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
                discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 30);
                startActivity(discoverableIntent);
            }
        });

        BluetoothAdapter bluetooth = BluetoothAdapter.getDefaultAdapter();
        if (bluetooth != null) {
            String status;
            if (bluetooth.isEnabled()) {
                String mydeviceaddress = bluetooth.getAddress();
                String mydevicename = bluetooth.getName();
                status = mydevicename + " : " + mydeviceaddress;
            } else {
                Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(turnOn, 0);
                status = "Bluetooth is not Enabled.";
            }

            //Toast.makeText(this, status, Toast.LENGTH_LONG).show();   // Continue with bluetooth setup.

            Set<BluetoothDevice> pairedDevices = bluetooth.getBondedDevices();

            ArrayList list = new ArrayList();

            for (BluetoothDevice bt : pairedDevices) list.add(bt.getName());
            Log.d("Searching", "seach");
            for (BluetoothDevice bt : pairedDevices) {
                if (bt.getName().equals("Solar Tracker")) {
                    Toast.makeText(getApplicationContext(), "FOUND", Toast.LENGTH_SHORT).show();
                }
                list.add(bt.getName());
            }
            //Toast.makeText(getApplicationContext(), list.toString(), Toast.LENGTH_SHORT).show();

        }
    }
}
