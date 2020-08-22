package araobp.theta.api

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import araobp.theta.Properties
import org.json.JSONObject

/**
 * Compound operations on Theta API v2
 */
class Operations(context: Context, props: Properties): ThetaClient(context, props) {

    interface IPictureCallback {
        fun onPictureTaken(bitmap: Bitmap?)
    }

    // Take a picture
    fun takePicture(callback: IPictureCallback) {

        handler?.post {
            var bitmap: Bitmap? = null

            // Fetch a fingerprint of the last capture
            val state = requestSync("/osc/state")
            var fingerprint = state?.getString("fingerprint")

            fingerprint?.let {

                // Take a picture
                val takePicture = JSONObject()
                        .put("name", "camera.takePicture")
                        .put("parameters", JSONObject().put("sessionId", sessionId))
                requestSync("/osc/commands/execute", takePicture)

                // Wait for status change
                var newPictureCaptured = false
                for (i in 0 until 5) {
                    Thread.sleep(200)
                    var stateFingerprint = fingerprint
                    val input = JSONObject().put("stateFingerprint", fingerprint)
                    requestSync("/osc/checkForUpdates", input)?.let {
                        stateFingerprint = it.getString("stateFingerprint")
                    }
                    if (fingerprint != stateFingerprint) {
                        fingerprint = stateFingerprint  // new fingerprint
                        newPictureCaptured = true
                        break
                    }
                }

                // Download the picture to a byte array
                if (newPictureCaptured) {

                    // Fetch the latest file's URI
                    val latestFileUri = requestSync("/osc/state")?.let {
                        it.getJSONObject("state").getString("_latestFileUri")
                    }

                    latestFileUri?.let {
                        val getImage = JSONObject()
                                .put("name", "camera.getImage")
                                .put("parameters", JSONObject().put("fileUri", latestFileUri))
                        val jpegImg = requestRawDataSync("/osc/commands/execute", getImage)
                        jpegImg?.let {
                            bitmap = BitmapFactory.decodeByteArray(it, 0, it.size)
                        }
                    }
                }
            }
            callback.onPictureTaken(bitmap)
        }
    }
}