import { View, Text, ActivityIndicator, StyleSheet, Button, Pressable, Image, TextInput } from 'react-native'
import React, { useEffect, useState } from 'react'
import { ScrollView } from 'react-native';
import ColorPicker from 'react-native-wheel-color-picker';
import { TouchableOpacity } from 'react-native';
import * as Network from 'expo-network';
import TcpSocket from 'react-native-tcp-socket';
import Loader from './loader';
// import InputColor from 'react-input-color';

const modes = {
    BASIC : "basic",
    STATIC : "static",
    AMBIENT : "ambient",
    DYNAMIC : "dynamic",
    DYNAMIC2 : "dynamic2"
}

const ModeButtons = ({selected, onPress}) => {
    const modes_arrays = Object.values(modes)
    return (
        <View>
            {modes_arrays.map((mode) => {
                return(
                    <View style={styles.modeButton} key = {mode} >
                        <Button title={mode} disabled={selected==mode} onPress={()=>onPress(mode)}/>
                    </View>
                )
            })}
        </View>
    )
}

const PowerSaving = ({checked, onToggled, addedDevices=[], removeDevice, addThisDevice, textInputVal, settextInputVal}) => {
    if(!checked)
    {
        return (
            <View style={styles.powerSaving}>
                <Button title='Power Saving (Click to enable)' color={"#FF1244"} onPress={onToggled}></Button>
            </View>
        )
    }
    return (
        <View>
            <View style={styles.powerSaving}>
                <Button title='Power Saving (Click to disable)' color={"#10CC12"} onPress={onToggled}></Button>
            </View>
            <Text style={styles.subtitle}>
                Added MAC Addresses :
            </Text>
            {addedDevices.map((device)=>{
                return(
                    <View style={styles.flex} key={device}>
                        <Text style={styles.deviceText}>{device}</Text>
                        <TouchableOpacity activeOpacity={0.1} onPress={() => {removeDevice(device)}} >
                            <Image
                            source={require("./../assets/dustbin.png")}
                            style={{ width: 25, height: 25 }}
                            />
                        </TouchableOpacity>
                    </View>
                )
            })}
            <View style={styles.flex}>
                <TextInput placeholder='MAC Address (found in settings)' style={styles.textInput} value={textInputVal} onChangeText={settextInputVal}/>
                <Button title='Add' fontSize={10} onPress={()=>{addThisDevice(textInputVal)}}></Button>
            </View>
            
        </View>
        
    )
    

}

const Remote = ({service}) => {
    const [primaryColor, setPrimaryColor] = React.useState({});
    const [secondaryColor, setsecondaryColor] = useState({})
    const [mode, setmode] = useState(modes.BASIC)
    const [powerSaving, setpowerSaving] = useState(false)
    const [addedDevices, setaddedDevices] = useState(["00:1B:44:11:3A:B7", "12:12:44:11:3A:B7", "00:1B:77:88:99:B7", "AA:BB:CC:11:3A:BB"]) // TODO :make this empty
    const [thisDevice, setthisDevice] = useState("")

    const [client, setclient] = useState(null)
    const [readyToRemote, setreadyToRemote] = useState(true)

    useEffect(() => {
        const options = {
            port: service.port,
            host: service.address,
            // localAddress: '127.0.0.1',
            reuseAddress: true,
            // localPort: 20000,
            // interface: "wifi",
          };
          
          // Create socket
          const client = TcpSocket.createConnection(options, () => {
            // Write on the socket
            console.log("connected")
          });
          
          client.on('data', function(data) {
            console.log('message was received', data);
            console.log(String.fromCharCode.apply(String, data))
            try
            {
                data = JSON.parse(data)
                
                let PrimaryRGB = data.PrimaryRGB
                let SecondaryRGB = data.SecondaryRGB

                setPrimaryColor(rgbToHex(PrimaryRGB[0], PrimaryRGB[1], PrimaryRGB[2]))
                setsecondaryColor(rgbToHex(SecondaryRGB[0], SecondaryRGB[1], SecondaryRGB[2]))
                setmode(data.light_mode)
                setpowerSaving(data.powersaving)

                setreadyToRemote(true)

            }
            catch(e)
            {
                console.log("JSON parsing failed", e)
            }
          });
          
          client.on('error', function(error) {
            console.log(error);
          });
          
          client.on('close', function(){
            console.log('Connection closed!');
          });

          setclient(client)
    }, [])
    

    const [textInputVal, settextInputVal] = useState("")

    const onPrimaryColorChangeComplete = (p_color)=> {
        setPrimaryColor(p_color)
        console.log(p_color)
        p_color = hexToRgb(p_color)
        console.log(p_color)
        client.write("cmd_pri " + p_color.r + " " + p_color.g + " " + p_color.b + "\n")
    }
    const onSecondaryColorChangeComplete = (p_color)=> {
        setsecondaryColor(p_color)
        console.log(p_color)
        p_color = hexToRgb(p_color)
        console.log(p_color)
        client.write("cmd_sec " + p_color.r + " " + p_color.g + " " + p_color.b + "\n")
    }

    const onModeChanged = (p_mode) => {
        console.log(p_mode)
        setmode(p_mode)
        client.write("cmd_mod " + p_mode + " \n")
    }

    const onPowerSavingToggled = () => {
        console.log("Power Saving toggled")
        setpowerSaving(!powerSaving)
        let x = !powerSaving
        if(x)
        {
            client.write("cmd_pow 1 \n")
        }
        else
        {
            client.write("cmd_pow 0 \n")
        }
        
    }

    const removeDevice = (p_device) => {
        let modifiedDevices = addedDevices
        modifiedDevices = modifiedDevices.filter((device)=>{return device!=p_device})
        setaddedDevices(modifiedDevices)
        // TODO : send to NodeMCU
    }

    const addThisDevice = (p_device) => {
        p_device = p_device.toUpperCase()
        let modifiedDevices = addedDevices
        modifiedDevices = modifiedDevices.filter((device)=>{return device!=p_device})
        modifiedDevices.push(p_device)
        setaddedDevices(modifiedDevices)
        // TODO : send to NodeMCU
    }

    if(!readyToRemote)
    {
        return (
            <View>
                <Loader message={"Connecting to Lights. Please wait..."} />
            </View>
        )
    }
  return (
    <View>
        <Text style={styles.title}>Light Controller</Text>
        <ScrollView style={styles.scroll}>
            
            <Text style={styles.subtitle}>Primary color</Text>
            <View style={styles.colorwrapper}>
                <ColorPicker 
                color={primaryColor} 
                onColorChangeComplete={onPrimaryColorChangeComplete}
                swatches={false}
                />
            </View>
            <Text style={styles.subtitle}>Secondary color</Text>
            <View style={styles.colorwrapper}>
                <ColorPicker 
                color={secondaryColor} 
                onColorChangeComplete={onSecondaryColorChangeComplete}
                swatches={false}
                />
            </View>
            <Text style={styles.subtitle}>Select Mode</Text>
            <ModeButtons selected={mode} onPress={onModeChanged}/>
            <PowerSaving 
            onToggled={onPowerSavingToggled} 
            checked={powerSaving} 
            addedDevices={addedDevices}
            removeDevice={removeDevice}
            addThisDevice={addThisDevice}
            textInputVal={textInputVal}
            settextInputVal={settextInputVal}
            />
        </ScrollView>
    </View>
  )
}

const styles = StyleSheet.create({
    title: {
      fontSize : 30,
      marginTop : 30,
      fontWeight : "bold",
      color : "#3e3e3e",
    },
    textInput:{
        paddingLeft:10,
        fontSize:12,
    }, 
    subtitle: {
        fontSize : 18,
        paddingLeft: 10,
        fontWeight : "bold",
        color : "#3e3e3e",
      },
    flex : {
        display : 'flex',
        flexDirection: "row",
        justifyContent : "space-between",
        alignItems:"center",
        marginBottom:20
    },
    deviceText: {
        fontSize : 14,
        paddingLeft: 10,
        color : "#3e3e3e",
      },
    colorwrapper : {
        padding:20
    },
    scroll : {
        marginTop: 5,
        marginBottom : 20,
        width: "100%",
    },
    modeWrapper : {
        display:'flex',
        flexDirection:'row',
        flexWrap:'wrap'
    },
    modeButton:{
        width : "auto",
        margin : 10,
    },
    powerSaving : {
        padding : 10,
    },
    enabledButton:{
        backgroundColor:"#00FF00",
    },
    disabledButton : {
        backgroundColor:"#FF0000",
    }
});

function componentToHex(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
}
  
  function rgbToHex(r, g, b) {
    return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

function hexToRgb(hex) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
      r: parseInt(result[1], 16),
      g: parseInt(result[2], 16),
      b: parseInt(result[3], 16)
    } : null;
}

export default Remote 