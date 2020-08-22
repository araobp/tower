package jp.araobp.aiwebcam.launcher

import android.content.Context
import android.net.wifi.WifiManager
import android.util.Log
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.SocketTimeoutException
import java.security.KeyStore
import kotlin.concurrent.thread

class AiWebCamDiscovery(val context: Context) {

    companion object {
        const val TAG = "AiWebCamDiscovery"
        const val AI_WEBCAM_PORT = 18082
        const val DISCOVERY_PORT = 18084
    }

    interface IDiscoveryCallback {
        fun onResult(err: Boolean, aiWebcamURL: String)
    }

    fun discover(callback: IDiscoveryCallback) {
        thread {
            var deviceId = ""
            var serverIpAddress = ""
            var err = true

            val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as WifiManager
            val multicastLock = wifiManager.createMulticastLock(TAG)
            multicastLock.setReferenceCounted(true)
            if (multicastLock != null && !multicastLock.isHeld) {
                multicastLock.acquire()
                val buf = ByteArray(6)
                val dp = DatagramPacket(buf, buf.size)
                val datagramSocket = DatagramSocket(DISCOVERY_PORT).apply {
                    broadcast = true
                    soTimeout = 5000  // 5 sec timeout
                }
                try {
                    datagramSocket.receive(dp)
                    datagramSocket.close()
                    deviceId = String(buf, 0, buf.size)
                    serverIpAddress = dp.address.hostAddress
                    Log.d(TAG, "AI Webcam discovered at $serverIpAddress")
                    err = false
                } catch (e: SocketTimeoutException) {
                    datagramSocket.close()
                    Log.d(TAG, "AI Webcam discovery timeout")
                } finally {
                    multicastLock.release()
                }
            } else {
                Log.d(TAG, "Multicast lock acquisition failed")
            }

            callback.onResult(err, "http://$serverIpAddress:$AI_WEBCAM_PORT/broadcast/$deviceId")
        }
    }
}
