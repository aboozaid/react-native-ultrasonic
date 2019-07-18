package reactnative.ultrasonic;

import android.util.Log;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.modules.core.DeviceEventManagerModule;

import java.nio.charset.Charset;

import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;
import rx.subscriptions.Subscriptions;

public class UltrasonicReceiver extends Ultrasonic {
    private Subscription frameSubscription = Subscriptions.empty();
    private ReactApplicationContext context;
    private String soundType;

    public UltrasonicReceiver(ReactApplicationContext context) {
        super(context);
        this.context = context;
    }

    @Override
    public void initialize(int sound, Promise promise) {
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
        runThread();
        promise.resolve(null);
    }

    private void runThread() {
        frameSubscription.unsubscribe();
        frameSubscription = FrameReceiverObservable.create(getApplicationContext(), soundType).subscribeOn(Schedulers.io()).observeOn(AndroidSchedulers.mainThread()).subscribe(buf -> {
            String payload = new String(buf, Charset.forName("UTF-8"));
            WritableMap data = Arguments.createMap();
            data.putString("payload", payload);
            context.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class).emit("onPayloadReceived", data);
            //Log.i("test", payload);
        }, error-> {
            WritableMap data = Arguments.createMap();
            data.putString("error", error.getMessage());
            //Log.i("test", error.getMessage());
            context.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class).emit("onErrorReceived", data);
        });
    }

    public void onResume() {
        runThread();
    }

    public void onPause() {
        frameSubscription.unsubscribe();
    }

    @Override
    public void onDestroy() {
        frameSubscription.unsubscribe();
    }
}
