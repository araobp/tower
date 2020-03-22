package araobp.theta

import android.app.Dialog
import android.graphics.Bitmap
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.widget.EditText
import araobp.theta.api.Operations
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private lateinit var mProps: Properties

    private var mOperations: Operations? = null

    private val mImages = mutableListOf<Bitmap?>(null, null)

    override fun onResume() {
        super.onResume()
        mOperations = Operations(this, mProps)
    }

    override fun onPause() {
        super.onPause()
        mOperations?.destroy()
        mOperations = null
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        mProps = Properties(this)

        buttonCapture.setOnClickListener {
            val callback = object : Operations.IPictureCallback {
                override fun onPictureTaken(bitmap: Bitmap?) {
                    mImages[0]?.let { prevImg ->
                        mImages[1] = prevImg
                        runOnUiThread {
                            imageView1.setImageBitmap(mImages[1])
                        }
                    }
                    mImages[0] = bitmap
                    runOnUiThread {
                        imageView0.setImageBitmap(mImages[0])
                    }
                }
            }
            mOperations?.takePicture(callback)
        }

        buttonSettings.setOnClickListener {
            val dialog = Dialog(this)
            dialog.setContentView(R.layout.settings)

            val editTextSSID = dialog.findViewById<EditText>(R.id.editTextSSID)
            editTextSSID.setText(mProps.ssid)

            val editTextPassword = dialog.findViewById<EditText>(R.id.editTextPassword)
            editTextPassword.setText(mProps.password)

            editTextSSID.addTextChangedListener(object : TextWatcher {
                override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) =
                        Unit

                override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) = Unit
                override fun afterTextChanged(p0: Editable?) {
                    mProps.ssid = editTextSSID.text.toString()
                }
            })

            editTextPassword.addTextChangedListener(object : TextWatcher {
                override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) =
                        Unit

                override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) = Unit
                override fun afterTextChanged(p0: Editable?) {
                    mProps.password = editTextPassword.text.toString()
                }
            })

            dialog.setOnDismissListener {
                mProps.save()
                mOperations?.destroy()
                mOperations = Operations(this@MainActivity, mProps)
            }

            dialog.show()
        }

    }
}
