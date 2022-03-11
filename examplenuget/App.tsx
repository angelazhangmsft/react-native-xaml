/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 * @flow strict-local
 */

import React, {useRef} from 'react';

import {
  findNodeHandle,
  SafeAreaView,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  useColorScheme,
  View,
} from 'react-native';

import {
  TextBox,
  MenuFlyout,
  MenuFlyoutItem,
  TextBlock,
  WinUI,
  MediaPlayerElement,
  Button,
  ContentDialogState,
  ContentDialog,
  ContentDialogButton,
  ContentDialogResult,
  Visibility,
  WinUIEnums,
} from 'react-native-xaml';
import {
  Colors,
  DebugInstructions,
  Header,
  LearnMoreLinks,
  ReloadInstructions,
} from 'react-native/Libraries/NewAppScreen';

const Section = ({children, title}) => {
  const isDarkMode = useColorScheme() === 'dark';
  return (
    <View style={styles.sectionContainer}>
      <Text
        style={[
          styles.sectionTitle,
          {
            color: isDarkMode ? Colors.white : Colors.black,
          },
        ]}>
        {title}
      </Text>
      <Text
        style={[
          styles.sectionDescription,
          {
            color: isDarkMode ? Colors.light : Colors.dark,
          },
        ]}>
        {children}
      </Text>
    </View>
  );
};

const App = () => {
  const isDarkMode = useColorScheme() === 'dark';

  const backgroundStyle = {
    backgroundColor: isDarkMode ? Colors.darker : Colors.lighter,
  };

  const menu = useRef<MenuFlyout>(null);
  const _tbRef = React.useRef<TextBlock>(null);
  const _mpRef = React.useRef<MediaPlayerElement>(null);
  const [x, setX] = React.useState("100");

  const [showState, setShowState] = React.useState(ContentDialogState.Hidden);
  const [visible, setVisible] = React.useState(Visibility.Visible);

  return (
    <SafeAreaView style={backgroundStyle}>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />
      <ScrollView
        contentInsetAdjustmentBehavior="automatic"
        style={backgroundStyle}>
        <Header />
        <View
          style={{
            backgroundColor: isDarkMode ? Colors.black : Colors.white,
          }}>
          {/* <TextBox
            text={`x`}
            onBeforeTextChanging={e => {
              if (e.nativeEvent.args) {
                setX(e.nativeEvent.args.newText);
              }
            }}
          /> */}

          <WinUI.InfoBar
            message="the message"
            title="Location coordinates:"
            isOpen={true}
            visibility={visible}
            onClosed={() => {
              setVisible(Visibility.Collapsed);
            }}
            severity={WinUIEnums.InfoBarSeverity.Success}
          />

          <Button content="Detect my coordinates" resources={{
            ButtonForeground: "#00fff1",
            ButtonForegroundPressed: "#2090ff",
          }} />

          <WinUI.ColorPicker onColorChanged={(e) => {
            console.log(e.nativeEvent);
          }} />

        </View>
      </ScrollView>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  sectionContainer: {
    marginTop: 32,
    paddingHorizontal: 24,
  },
  sectionTitle: {
    fontSize: 24,
    fontWeight: '600',
  },
  sectionDescription: {
    marginTop: 8,
    fontSize: 18,
    fontWeight: '400',
  },
  highlight: {
    fontWeight: '700',
  },
});

export default App;
