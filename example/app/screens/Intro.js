import React, { Component } from 'react';
import { View, Text, TouchableOpacity, PermissionsAndroid } from 'react-native';

export default class Intro extends Component {
  static navigationOptions = {
      title: 'Send data over ultrasonic'
  }
  async componentDidMount() {
    await this.requestCameraPermission();
  }

  async requestCameraPermission() {
    try {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.RECORD_AUDIO,
        {
          title: 'Permission to use your audio and microphone',
          message:
            'We need access to your microphone and audio ' +
            'so you send and receive messages.',
          buttonNeutral: 'Ask Me Later',
          buttonNegative: 'Cancel',
          buttonPositive: 'OK',
        },
      );
      if (granted === PermissionsAndroid.RESULTS.GRANTED) {
        console.log('You can use the camera');
      } else {
        console.log('Camera permission denied');
      }
    } catch (err) {
      console.warn(err);
    }
  }

  render() {
    return (
      <View style = {{flex: 1, flexDirection: 'column'}}>
        <View style = {{flex: 1, flexDirection: 'row', alignItems: 'flex-end', justifyContent: 'center'}}>
            <TouchableOpacity style = {{backgroundColor: '#6C55AE', padding: 10, margin: 12, width: '50%'}} onPress = {() => this.props.navigation.navigate('sender')} >
                <Text style = {{textAlign: 'center', color: '#fff'}}>Send</Text>
            </TouchableOpacity>
        </View>
        <View style = {{flex: 1, flexDirection: 'row', alignItems: 'flex-start', justifyContent: 'center'}}>
            <TouchableOpacity style = {{backgroundColor: '#6C55AE', padding: 10, margin: 12, width: '50%'}} onPress = {() => this.props.navigation.navigate('receiver')}>
                <Text style = {{textAlign: 'center', color: '#fff'}}>Receive</Text>
            </TouchableOpacity>
        </View>
      </View>
    );
  }
}
