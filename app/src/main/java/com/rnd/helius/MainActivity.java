package com.rnd.helius;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Set;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ldr0Button = (Button) findViewById(R.id.ldr0);
        ldr0Button.setText((int)(Math.random() * 1000) + "");
        ldr1Button = (Button) findViewById(R.id.ldr1);
        ldr1Button.setText((int)(Math.random() * 1000) + "");
        ldr2Button = (Button) findViewById(R.id.ldr2);
        ldr2Button.setText((int)(Math.random() * 1000) + "");
        ldr3Button = (Button) findViewById(R.id.ldr3);
        ldr3Button.setText((int)(Math.random() * 1000) + "");

        powerBar = (ProgressBar) findViewById(R.id.powerBar);
        powerBar.setProgress((int)(Math.random() * 100));

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

        BluetoothAdapter bluetooth = BluetoothAdapter.getDefaultAdapter();
            if (bluetooth != null &&!bluetooth.isEnabled()) {
                Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(turnOn, 0);
            }
            Set<BluetoothDevice> pairedDevices = bluetooth.getBondedDevices();
            ArrayList list = new ArrayList();
            for(BluetoothDevice bt : pairedDevices) list.add(bt.getName());
            Toast.makeText(getApplicationContext(), list.toString() ,Toast.LENGTH_SHORT).show();

        }
}
