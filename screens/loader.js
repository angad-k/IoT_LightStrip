import { View, Text, ActivityIndicator, StyleSheet } from 'react-native'
import React from 'react'

const Loader = ({message}) => {
  return (
    <View>
        <Text style={styles.message}>{message}</Text>
        <ActivityIndicator size="large" />
    </View>
  )
}

const styles = StyleSheet.create({
    message: {
      color : "#696969",
      fontSize : 18,
      padding : 20,
      textAlign : "center",
      fontWeight : "bold",
    },
  });

export default Loader 