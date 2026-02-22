

example: check what overwriting sound_effects_enabled value to 0 even if 1 in overlay -> ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK'/frameworks/base/core/java/android/provider/Settings.java:


```
D/AudioProfileService( 1393): handleMessage what = 5, name = sound_effects_enabled, value = 0 or null
D/Provider/Settings( 1393): put string name = sound_effects_enabled , value = 0 userHandle = 0
```


![Debug code throw caller](/image/Pasted%20image%2020260223005222.png)


```

D/20260222_debug_sound_setting( 1393): value---->0
D/20260222_debug_sound_setting( 1393): java.lang.Throwable
D/20260222_debug_sound_setting( 1393): 	at android.provider.Settings$NameValueCache.putStringForUser(Settings.java:904)
D/20260222_debug_sound_setting( 1393): 	at android.provider.Settings$System.putStringForUser(Settings.java:1195)
D/20260222_debug_sound_setting( 1393): 	at android.provider.Settings$System.putIntForUser(Settings.java:1300)
D/20260222_debug_sound_setting( 1393): 	at android.provider.Settings$System.putInt(Settings.java:1294)
D/20260222_debug_sound_setting( 1393): 	at com.mediatek.audioprofile.AudioProfileService$AudioProfileHandler.handleMessage(AudioProfileService.java:3420)
D/20260222_debug_sound_setting( 1393): 	at android.os.Handler.dispatchMessage(Handler.java:110)
D/20260222_debug_sound_setting( 1393): 	at android.os.Looper.loop(Looper.java:193)
D/20260222_debug_sound_setting( 1393): 	at com.mediatek.audioprofile.AudioProfileService$OverrideSystemThread.run(AudioProfileService.java:3363)
```
