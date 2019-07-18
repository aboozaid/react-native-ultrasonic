import {createStackNavigator, createAppContainer} from 'react-navigation';
import Sender from './screens/Sender';
import Receiver from './screens/Receiver';
import Intro from './screens/Intro';


const mainTabs = createStackNavigator({
    app: {screen: Intro},
    sender: {screen: Sender},
    receiver: {screen: Receiver},
});

export default createAppContainer(mainTabs);
