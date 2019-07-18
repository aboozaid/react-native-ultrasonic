package reactnative.ultrasonic;

import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;

public abstract class Ultrasonic extends ReactApplicationContext {

    public Ultrasonic(ReactApplicationContext context) {
        super(context);
    }

    public abstract void initialize(int sound, Promise promise);

    public abstract void onDestroy();
}
