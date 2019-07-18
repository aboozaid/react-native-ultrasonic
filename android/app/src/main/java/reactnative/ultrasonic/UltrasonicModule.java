package reactnative.ultrasonic;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.LifecycleEventListener;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.ReadableMap;
import com.facebook.react.bridge.WritableMap;

import java.util.HashMap;
import java.util.Map;

public class UltrasonicModule extends ReactContextBaseJavaModule implements LifecycleEventListener {
    private static String module = "ultrasonic";
    private UltrasonicSender sender;
    private UltrasonicReceiver receiver;
    private ReactApplicationContext reactContext;
    public interface sounds {
        int CHANNEL0_7K = 0;
        int CHANNEL1_7K = 1;
        int ULTRA_EXPER = 2;
    }

    public UltrasonicModule(ReactApplicationContext reactContext) {
        super(reactContext);
        reactContext.addLifecycleEventListener(this);
        this.reactContext = reactContext;
    }

    @Override
    public String getName() {
        return module;
    }

    @Override
    public void onHostResume() {
        if(receiver != null) {
            receiver.onResume();
        }
    }

    @Override
    public void onHostPause() {
        if(receiver != null) {
            receiver.onPause();
        }
    }

    @Override
    public void onHostDestroy() {
       if(sender != null) {
           sender.onDestroy();
       }
       if(receiver != null) {
           receiver.onDestroy();
       }
    }

    @Override
    public Map<String, Object> getConstants() {
        final Map<String, Object> constants = new HashMap<>();

        WritableMap sound_type = Arguments.createMap();
        sound_type.putInt("channel_7k_0", sounds.CHANNEL0_7K);
        sound_type.putInt("channel_7k_1", sounds.CHANNEL1_7K);
        sound_type.putInt("ultrasonic", sounds.ULTRA_EXPER);


        constants.put("sounds", sound_type);

        return constants;
    }

    @ReactMethod
    public void initialize(ReadableMap data, Promise promise) {
        sender = new UltrasonicSender(reactContext);
        if(data.hasKey("sound"))
            sender.initialize(data.getInt("sound"), promise);
        else
            sender.initialize(sounds.ULTRA_EXPER, promise);
    }

    @ReactMethod
    public void send(ReadableMap data, Promise promise) {
        if(sender != null) {
            sender.send(data.getString("payload"), promise);
        } else {
            promise.reject("error", "Please call initialize function first!");
        }
    }

    @ReactMethod
    public void receive(ReadableMap data, Promise promise) {
        receiver = new UltrasonicReceiver(reactContext);
        if(data.hasKey("sound"))
            receiver.initialize(data.getInt("sound"), promise);
        else
            promise.reject("error", "No sound type set please specify one");
    }
}
