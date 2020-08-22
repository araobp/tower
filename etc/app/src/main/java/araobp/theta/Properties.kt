package araobp.theta

import android.content.Context

class Properties(val context: Context) {

    companion object {
        const val PREFS_NAME = "theta"
    }

    var ssid = ""
    var password = ""

    init {
        load()
    }

    /**
     * Load local preferences
     */
    fun load() {
        val prefs = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        ssid = prefs.getString("ssid", "").toString()
        password = prefs.getString("password", "").toString()
    }

    /**
     * Sage local preferences
     */
    fun save() {
        val editor = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE).edit()
        editor.putString("ssid", ssid)
        editor.putString("password", password)
        editor.apply()
    }
}