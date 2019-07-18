import React, { Component } from 'react';
import { View, Text, DeviceEventEmitter, TouchableOpacity, ListView } from 'react-native';
import ultrasonic from 'react-native-ultrasonic';

export default class Receiver extends Component {
    static navigationOptions = {
        title: 'Received Data'
    }
    constructor(props) {
        super(props);
        this.ds = new ListView.DataSource({rowHasChanged: (r1, r2) => r1 !== r2})
        const payloads = [];
        this.state = {
            payloads,
            dataSource: this.ds.cloneWithRows(payloads)
        }
    }
    componentDidMount() {
        ultrasonic.receive({sound: ultrasonic.sounds.ultrasonic}).then(() => {
            // initialized
        }).catch(error => {
            alert(error.error);
        });

        DeviceEventEmitter.addListener('onPayloadReceived', this.onPayloadReceived);
        DeviceEventEmitter.addListener('onErrorReceived', this.onPayloadReceived);
    }

    componentWillUnmount() {
        DeviceEventEmitter.removeListener('onPayloadReceived', this.onPayloadReceived);
        DeviceEventEmitter.removeListener('onErrorReceived', this.onPayloadReceived);
    }

    onPayloadReceived = ({payload}) => {
        const payloads = [...this.state.payloads, {msg: payload}];
        this.setState({payloads, dataSource: this.ds.cloneWithRows(payloads)});
    }

    onErrorReceived = ({error}) => {
        alert(error);
    }

  render() {
    return (
        <View style = {{flex: 1, flexDirection: 'column'}}>
        <View style = {{flex: 3, flexDirection: 'row'}}>
            <ListView 
                dataSource = {this.state.dataSource}
                enableEmptySections
                renderRow = {(payload) => {
                    return (
                        <View style = {{margin: 15, flexDirection: 'column', flex: 1}}>
                           <View style = {{flexDirection: 'row', flex: 1}}>
                                <Text style = {{color: '#6C55AE', fontSize: 24, fontWeight: "300"}}>{payload.msg}</Text>
                            </View>
                        </View>
                    )
                }}
            />
        </View>
        <View style = {{flex: 1, flexDirection: 'row', alignItems: 'flex-start', justifyContent: 'center'}}>
            <TouchableOpacity style = {{backgroundColor: '#6C55AE', padding: 10, margin: 12, width: '90%'}}>
                <Text style = {{textAlign: 'center', color: '#fff'}}>You should see received payloads from others</Text>
            </TouchableOpacity>
        </View>
      </View>
    );
  }
}
