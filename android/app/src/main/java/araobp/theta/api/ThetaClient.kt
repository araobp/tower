package araobp.theta.api

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.net.wifi.WifiNetworkSpecifier
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.widget.Toast
import araobp.theta.Properties
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaTypeOrNull
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import java.io.IOException


/**
 * RICHO Theta REST API client
 *
 * [RICHO Theta API v2 reference] https://api.ricoh/docs/theta-web-api-v2/
 */
open class ThetaClient(val context: Context, val props: Properties) {

    companion object {
        val TAG: String = this::class.java.simpleName
        const val HOST = "192.168.1.1"
        const val PORT = "80"
        val MEDIA_TYPE = "application/json; charset=utf-8".toMediaTypeOrNull()
    }

    private var mHandlerThread: HandlerThread? = HandlerThread("sync").apply { start() }
    internal var handler: Handler? = mHandlerThread?.let {Handler(it.looper)}

    private var mClient: OkHttpClient? = null

    var sessionId: String? = null

    init {
        connect()
    }

    /**
     * Connect to Theta via WiFi
     */
    private fun connect() {

        val wifiNetworkSpecifier = WifiNetworkSpecifier.Builder()
                .setSsid(props.ssid)
                .setWpa2Passphrase(props.password)
                .build()

        val networkRequestBuilder = NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .setNetworkSpecifier(wifiNetworkSpecifier)

        val networkRequest = networkRequestBuilder.build()
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val networkCallback = object : ConnectivityManager.NetworkCallback() {
            override fun onAvailable(network: Network) {
                super.onAvailable(network)
                Log.d(TAG, "onAvailable")
                mClient = OkHttpClient.Builder()
                        .socketFactory(network.socketFactory)
                        .build()
                sessionId()
            }
        }
        cm.requestNetwork(networkRequest, networkCallback)
    }

    private interface IReceiver {
        fun onResponse(output: JSONObject?)
        fun onFailure(e: IOException)
    }

    fun requestSync(api: String, input: JSONObject = JSONObject()): JSONObject? {
        val url = "http://$HOST:$PORT$api"
        val body = input?.let { it.toString().toRequestBody(MEDIA_TYPE) }
        val request = Request.Builder().apply {
            url(url)
            post(body)
        }.build()
        val response = mClient?.newCall(request)?.execute()
        return response?.let {
            it.body?.let { body -> JSONObject(body.string()) }
        }
    }

    fun requestRawDataSync(api: String, input: JSONObject? = null): ByteArray? {
        val url = "http://$HOST:$PORT$api"
        val body = input?.let { it.toString().toRequestBody(MEDIA_TYPE) }
        val request = Request.Builder().apply {
            url(url)
            body?.let {post(body)}
        }.build()
        val response = mClient?.newCall(request)?.execute()
        return response?.body?.bytes()
    }

    private fun requestAsync(api: String, input: JSONObject, receiver: IReceiver) {
        val url = "http://$HOST:$PORT$api"
        val body = input.toString().toRequestBody(MEDIA_TYPE)
        val request = Request.Builder()
                .url(url)
                .post(body)
                .build()
        val callback = object : Callback {
            override fun onResponse(call: Call, response: Response) {
                val body = JSONObject(response.body?.string())
                val code = response.code
                    receiver.onResponse(body)
            }
            override fun onFailure(call: Call, e: IOException) {
                receiver.onFailure(e)
            }
        }
        mClient?.newCall(request)?.enqueue(callback)
    }

    private fun sessionId() {
        val input = JSONObject()
                .put("name", "camera.startSession")
                .put("parameters", JSONObject())
        val receiver = object : IReceiver {
            override fun onResponse(output: JSONObject?) {
                output?.let {
                    sessionId = it
                            .getJSONObject("results")
                            .getString("sessionId")
                    Log.d(TAG, "sessionId: $sessionId")

                    //Toast.makeText(context, "sessionId: $sessionId", Toast.LENGTH_LONG).show()
                }
            }
            override fun onFailure(e: IOException) {
                Log.d(TAG, e.toString())
            }
        }
        requestAsync("/osc/commands/execute", input, receiver)
    }


    fun destroy() {
        mHandlerThread?.quitSafely()
        mHandlerThread?.join()
        handler = null
        mHandlerThread = null
    }
}