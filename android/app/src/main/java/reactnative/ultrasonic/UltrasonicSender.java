package reactnative.ultrasonic;

import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;

import org.quietmodem.Quiet.FrameTransmitter;
import org.quietmodem.Quiet.FrameTransmitterConfig;
import org.quietmodem.Quiet.ModemException;

import java.io.IOException;

public class UltrasonicSender extends Ultrasonic {
    private FrameTransmitter mTransmitter;

    public UltrasonicSender(ReactApplicationContext context) {
        super(context);
    }

    @Override
    public void initialize(int sound, Promise promise) {
        String soundType;
        switch (sound) {
            case UltrasonicModule.sounds.CHANNEL0_7K:
                soundType = "audible-7k-channel-0";
                break;
            case UltrasonicModule.sounds.CHANNEL1_7K:
                soundType = "audible-7k-channel-1";
                break;
            default:
                soundType = "ultrasonic-experimental";
                break;
        }

        if(mTransmitter != null) {
            mTransmitter = null;
        }

        FrameTransmitterConfig transmitterConfig;

        try {
            transmitterConfig = new FrameTransmitterConfig(this, soundType);
            mTransmitter = new FrameTransmitter(transmitterConfig);
            promise.resolve(null);
        } catch (IOException e) {
            promise.reject("error", e.getMessage());
        } catch (ModemException e) {
            promise.reject("error", e.getMessage());
        }
    }

    public void send(String payload, Promise promise) {
        try {
            mTransmitter.send(payload.getBytes());
            promise.resolve(null);
        } catch (IOException e) {
            // our message might be too long or the transmit queue full
            promise.reject("error", e.getMessage());
        }
    }

    @Override
    public void onDestroy() {
        if(mTransmitter != null) {
            mTransmitter.close();
        }
    }
}
