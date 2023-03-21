import { StatusBar } from 'expo-status-bar';
import { useEffect, useState } from 'react';
import { StyleSheet, Text, View } from 'react-native';
import Loader from './screens/loader';
import Remote from './screens/remote';
import Zeroconf from 'react-native-zeroconf'

const zeroconf = new Zeroconf()

export default function App() {
  const [service, setservice] = useState({})
  const [lightsFound, setlightsFound] = useState(false)

  useEffect(() => {
    zeroconf.on('start', () => console.log('The scan has started.'))
    zeroconf.on('stop', () => console.log('The scan has stopped.'))
    zeroconf.on('resolved', (props) => console.log('service resolved', props))
    console.log(zeroconf)
    

    const startScanning = () => {
      try {
        zeroconf.scan("arduino", "tcp", "local.");
      } catch (err) {
        console.warn(err);
        setTimeout(() => {
          startScanning()
        }, 1000)
      }
    }

    startScanning()
    // zeroconf.scan(type = 'http', protocol = 'tcp', domain = 'local.')
   
  }, [])
  

  if(lightsFound)
  {
    return (
      <View style={styles.container}>
        <Remote />
        <StatusBar style="auto" />
      </View>
    );
  }
  else
  {
    return (
      <View style={styles.container}>
        <Loader message={"Trying to find suitable device on the network. Please wait..."} />
        <StatusBar style="auto" />
      </View>
    );
  }
  
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#fff',
    alignItems: 'center',
    justifyContent: 'center',
  },
});
