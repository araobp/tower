package araobp.theta

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.net.wifi.WifiNetworkSpecifier
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import okhttp3.MediaType.Companion.toMediaTypeOrNull
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject


/**
 * RICHO Theta REST API client
 *
 * [RICHO Theta API v2 reference] https://api.ricoh/docs/theta-web-api-v2/
 */

class ThetaClient(val context: Context, val props: Properties) {

    companion object {
        val TAG: String = this::class.java.simpleName
        const val HOST = "192.168.1.1"
        const val PORT = "80"
    }

    private var mClient: OkHttpClient? = null

    private var mHandlerThread: HandlerThread? = HandlerThread("sync").apply { start() }
    private var mHandler: Handler? = mHandlerThread?.let {Handler(it.looper)}

    init {
        connectToWiFi()
    }

    private fun connectToWiFi() {

        val wifiNetworkSpecifier = WifiNetworkSpecifier.Builder()
            .setSsid(props.ssid)
            .setWpa2Passphrase(props.password)
            .build()

        val networkRequestBuilder = NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
            .setNetworkSpecifier(wifiNetworkSpecifier)

        val networkRequest = networkRequestBuilder.build()
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val networkCallback = object: ConnectivityManager.NetworkCallback() {
            override fun onAvailable(network: Network) {
                super.onAvailable(network)
                Log.d(TAG, "onAvailable")
                mClient = OkHttpClient.Builder()
                    .socketFactory(network.socketFactory)
                    .build()
                capture()
            }
        }
        cm.requestNetwork(networkRequest, networkCallback)
    }

    fun capture() {
        val jsonObject = JSONObject()
        jsonObject.put("name", "camera.startSession")
        jsonObject.put("parameters", JSONObject())

        val mediaType  = "application/json; charset=utf-8".toMediaTypeOrNull()
        val url = "http://$HOST:$PORT/osc/commands/execute"
        val body = jsonObject.toString().toRequestBody(mediaType)
        val request = Request.Builder()
            .url(url)
            .post(body)
            .build()
        Log.d(TAG, url)
        Log.d(TAG, jsonObject.toString())
        mHandler?.post {
            try {
                val response = mClient?.newCall(request)?.execute()
                response?.let {
                    val bodyString = it.body?.string()
                    bodyString?.let { bodyString ->
                        val jsonObject = JSONObject(bodyString)
                        val sessionId = jsonObject
                            .getJSONObject("results")
                            .getString("sessionId")
                        Log.d(TAG, "sessionId: $sessionId")
                    }
                }
            } catch (e: Exception) {
                Log.d(TAG, e.printStackTrace().toString())
            }
        }
    }

    fun destroy() {
        mHandlerThread?.quitSafely()
        mHandlerThread?.join()
        mHandler = null
        mHandlerThread = null
    }

}