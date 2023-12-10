import 'dart:async';
import 'dart:convert' show utf8;

import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/ui/firebase_animated_list.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:firebase_database/firebase_database.dart';

class SensorPage extends StatefulWidget {
  const SensorPage({Key key, this.device}) : super(key: key);
  final BluetoothDevice device;

  @override
  _SensorPageState createState() => _SensorPageState();
}

class _SensorPageState extends State<SensorPage> {
  final String SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
  final String CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
  bool isReady;
  Stream<List<int>> stream;

  final refee= FirebaseDatabase.instance.reference('unhealthy');


  @override
  void initState() {
    super.initState();
    isReady = false;
    connectToDevice();
  }

  connectToDevice() async {
    if (widget.device == null) {
      _Pop();
      return;
    }

    new Timer(const Duration(seconds: 15), () {
      if (!isReady) {
        disconnectFromDevice();
        _Pop();
      }
    });

    await widget.device.connect();
    discoverServices();
  }

  disconnectFromDevice() {
    if (widget.device == null) {
      _Pop();
      return;
    }

    widget.device.disconnect();
  }

  discoverServices() async {
    if (widget.device == null) {
      _Pop();
      return;
    }

    List<BluetoothService> services = await widget.device.discoverServices();
    services.forEach((service) {
      if (service.uuid.toString() == SERVICE_UUID) {
        service.characteristics.forEach((characteristic) {
          if (characteristic.uuid.toString() == CHARACTERISTIC_UUID) {
            characteristic.setNotifyValue(!characteristic.isNotifying);
            stream = characteristic.value;

            setState(() {
              isReady = true;
            });
          }
        });
      }
    });

    if (!isReady) {
      _Pop();
    }
  }

  Future<bool> _onWillPop() {
    return showDialog(
        context: context,
        builder: (context) =>
            new AlertDialog(
              title: Text('Are you sure?'),
              content: Text('Do you want to disconnect device and go back?'),
              actions: <Widget>[
                new FlatButton(
                    onPressed: () => Navigator.of(context).pop(false),
                    child: new Text('No')),
                new FlatButton(
                    onPressed: () {
                      disconnectFromDevice();
                      Navigator.of(context).pop(true);
                    },
                    child: new Text('Yes')),
              ],
            ) ??
            false);
  }

  _Pop() {
    Navigator.of(context).pop(true);
  }

  String _dataParser(List<int> dataFromDevice) {
    return utf8.decode(dataFromDevice);
  }

  String getCategoryPm25(String currenValue) {
    Future.delayed(Duration(seconds: 30));
    double conpm = double.parse(currenValue);
    String rescon = "Good";
    if ((conpm > 12) && (conpm < 35.4)) {
      rescon = "Moderate";
    }
    if ((conpm > 35.4) && (conpm < 55.4)) {
      rescon = "Unhealthy for Sensitive Groups";
    }
    if ((conpm > 55.4) && (conpm < 150.4)) {
      rescon = "Unhealthy";
    }
    if ((conpm > 150.4) && (conpm < 250.4)) {
      rescon = "Very Unhealthy";
    }
    if ((conpm > 250.5)) {
      rescon = "Hazardous";
    }

    return rescon;
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      onWillPop: _onWillPop,
      child: Scaffold(
        appBar: AppBar(
          title: Text('Optical Dust Sensor'),
        ),
        body: Container(
            child: !isReady
                ? Center(
                    child: Text(
                      "Waiting...",
                      style: TextStyle(fontSize: 24, color: Colors.red),
                    ),
                  )
                : Container(
                    child: StreamBuilder<List<int>>(
                      stream: stream,
                      builder: (BuildContext context,
                          AsyncSnapshot<List<int>> snapshot) {
                        if (snapshot.hasError) return Text('Error');

                        if (snapshot.connectionState ==
                            ConnectionState.active) {
                          String currentValue = _dataParser(snapshot.data);

                          return Center(
                              child: Column(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: <Widget>[
                              Expanded(
                                flex: 1,
                                child: Column(
                                    mainAxisAlignment: MainAxisAlignment.center,
                                    children: <Widget>[
                                      Text('Air Quality Level',
                                          style: TextStyle(fontSize: 14)),
                                      Text(getCategoryPm25(currentValue),
                                          style: TextStyle(
                                              fontWeight: FontWeight.bold,
                                              fontSize: 24)),
                                    ]),
                              ),
                              Expanded(
                                flex: 1,
                                child: Column(
                                    mainAxisAlignment: MainAxisAlignment.center,
                                    children: <Widget>[
                                      Text('PM Density ',
                                          style: TextStyle(
                                              fontWeight: FontWeight.bold,
                                              fontSize: 24)),
                                      Text(currentValue + ' ug/m3',
                                          style: TextStyle(
                                              fontWeight: FontWeight.bold,
                                              fontSize: 24)),
                                    ]),
                              ),
                              Expanded(
                                flex: 1,
                                child: Column(
                                    mainAxisAlignment: MainAxisAlignment.center,
                                    children: <Widget>[]),
                              ),
                              Expanded(child: FirebaseAnimatedList(query: ,)
                            ],
                          ));
                        } else {
                          return Text('Check the stream');
                        }
                      },
                    ),
                  )),
      ),
    );
  }
}