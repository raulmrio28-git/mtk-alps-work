
```
V/MediaProvider( 2069): insertInternal>>>: uri=content://media/internal/audio/media, match=100, initValues=album_artist=null genre=null date_modified=1771847967 album=notifications track=0 is_ringtone=false is_alarm=false is_music=false is_podcast=false compilation=0 is_drm=false composer=null title=msg_mel02 duration=3763 is_notification=true mime_type=audio/midi _size=423 artist=<unknown> _data=/system/media/audio/notifications/msg_mel02.mid
V/MediaProvider( 2069): insertFile>>>: uri=content://media/internal/audio/media, mediaType=2, values=album_artist=null date_modified=1771847967 album=notifications track=0 is_ringtone=false is_alarm=false is_music=false is_podcast=false compilation=0 is_drm=false composer=null title=msg_mel02 duration=3763 is_notification=true mime_type=audio/midi _size=423 artist=<unknown> _data=/system/media/audio/notifications/msg_mel02.mid
D/MediaProvider_20260224_changename( 2069): java.lang.Exception
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaProvider.insertFile(MediaProvider.java:3433)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaProvider.insertInternal(MediaProvider.java:3920)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaProvider.bulkInsert(MediaProvider.java:3252)
D/MediaProvider_20260224_changename( 2069): 	at android.content.ContentProvider$Transport.bulkInsert(ContentProvider.java:246)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaInserter.flush(MediaInserter.java:93)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaInserter.flushAll(MediaInserter.java:76)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaScanner$MyMediaScannerClient.endFile(MediaScanner.java:1151)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaScanner$MyMediaScannerClient.doScanFile(MediaScanner.java:660)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaScanner$MyMediaScannerClient.scanFile(MediaScanner.java:591)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaScanner.processDirectory(Native Method)
D/MediaProvider_20260224_changename( 2069): 	at android.media.MediaScanner.scanDirectories(MediaScanner.java:1644)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaScannerService.scan(MediaScannerService.java:125)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaScannerService.handleScanRequest(MediaScannerService.java:356)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaScannerService.access$100(MediaScannerService.java:61)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaScannerService$ServiceHandler.handleMessage(MediaScannerService.java:287)
D/MediaProvider_20260224_changename( 2069): 	at android.os.Handler.dispatchMessage(Handler.java:110)
D/MediaProvider_20260224_changename( 2069): 	at android.os.Looper.loop(Looper.java:193)
D/MediaProvider_20260224_changename( 2069): 	at com.android.providers.media.MediaScannerService.run(MediaScannerService.java:232)
D/MediaProvider_20260224_changename( 2069): 	at java.lang.Thread.run(Thread.java:848)
```

