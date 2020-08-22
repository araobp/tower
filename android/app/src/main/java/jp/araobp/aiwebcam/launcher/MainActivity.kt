package jp.araobp.aiwebcam.launcher

import android.content.ActivityNotFoundException
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity() {

    private lateinit var mDiscovery: AiWebCamDiscovery
    private var mAiWebcamUrl: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mDiscovery = AiWebCamDiscovery(this)

        buttonLaunch.setOnClickListener {
            mAiWebcamUrl?.let {
                val intent = Intent(Intent.ACTION_VIEW, Uri.parse(it))
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                intent.setPackage("com.android.chrome")
                try {
                    startActivity(intent)
                } catch (ex: ActivityNotFoundException) {
                    textViewInfo.append("Unable to launch Chrome browser!")
                }
            }
        }
    }

    override fun onResume() {
        super.onResume()
        textViewInfo.append("Discovering AI Webcam...\n")
        mDiscovery.discover(
            object : AiWebCamDiscovery.IDiscoveryCallback {
                override fun onResult(err: Boolean, aiWebcamURL: String) {
                    if (err) {
                        textViewInfo.append("Discovery failed\n")
                    } else {
                        mAiWebcamUrl = aiWebcamURL
                        textViewInfo.append("Discoverd: $aiWebcamURL\n")
                    }
                }
            }
        )
    }
}