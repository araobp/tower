package araobp.theta

import android.app.Dialog
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.widget.EditText
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.settings.*

class MainActivity : AppCompatActivity() {

    private lateinit var mProps: Properties

    private lateinit var mThetaClient: ThetaClient

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        mProps = Properties(this)
        mThetaClient = ThetaClient(this, mProps)

        buttonCapture.setOnClickListener {
            mThetaClient.capture()
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
                mThetaClient.destroy()
                mThetaClient = ThetaClient(this, mProps)
            }

            dialog.show()
        }

    }
}
