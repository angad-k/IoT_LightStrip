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
    zeroconf.on('resolved', (props) => {
      console.log('service resolved', props)
      if(props.name === "LightStrip")
      {
        setservice({
          address : props.addresses[0],
          port : props.port
        })
        setlightsFound(true)
      }
    })
    zeroconf.on('found', (props) => console.log('service found', props))
    // zeroconf.scan(undefined, undefined, undefined, 'DNSSD')
    zeroconf.scan()   
    console.log("reached")
  }, [])
  

  if(lightsFound)
  {
    return (
      <View style={styles.container}>
        <Remote service={service}/>
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
