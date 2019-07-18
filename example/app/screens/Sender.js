import React, { Component } from 'react';
import { View, Text, TextInput, TouchableOpacity, Picker } from 'react-native';
import ultrasonic from 'react-native-ultrasonic';

export default class Sender extends Component {
    static navigationOptions = {
        title: 'Send Data'
    }
 state = {
     value : '2'
 }

 componentDidMount() {
    ultrasonic.initialize({sound: ultrasonic.sounds.ultrasonic}).then(() => {
        // initialized
     }).catch(error => {
         alert(error.error);
     })
 }
 
  onChange = (itemValue) => {
    this.setState({value: itemValue});

    /*ultrasonic.initialize({sound: ultrasonic.sounds.ultrasonic}).then(() => {
        // initialized
     }).catch(error => {
         alert(error.error);
     })*/

     ultrasonic.initialize({sound: parseInt(itemValue)}).then(() => {
        // initialized
     }).catch(error => {
         alert(error.error);
     })
  }
  onSend = () => {
    const { payload } = this.state;
    ultrasonic.send({payload}).then(() => {
        // message has been sent
    }).catch(error => {
        alert(error);
    })
  }

  render() {
    return (
        <View style = {{flex: 1, flexDirection: 'column'}}>
        <View style = {{flex: 1, flexDirection: 'row', alignItems: 'flex-end', justifyContent: 'center'}}>
            <TextInput placeholder = 'message' onChangeText = {(payload) => this.setState({payload})} style = {{borderWidth: 1, borderColor: '#eee', width: '90%'}}  />
        </View>
        <View style = {{flex: .5, flexDirection: 'row', alignItems: 'flex-start', justifyContent: 'center'}}>
            <Picker
                selectedValue={this.state.value}
                style={{height: 50, width: '90%'}}
                onValueChange={this.onChange}>
                <Picker.Item label="audible-7k-channel-0" value="0" />
                <Picker.Item label="audible-7k-channel-1" value="1" />
                <Picker.Item label="ultrasonic" value="2" />
            </Picker>
        </View>
        <View style = {{flex: 1, flexDirection: 'row', alignItems: 'flex-start', justifyContent: 'center'}}>
            <TouchableOpacity style = {{backgroundColor: '#6C55AE', padding: 10, margin: 12, width: '90%'}} onPress = {this.onSend}>
                <Text style = {{textAlign: 'center', color: '#fff'}}>Send</Text>
            </TouchableOpacity>
        </View>
      </View>
    );
  }
}
