package com.example.rebeat

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Bundle
import android.view.View
import android.webkit.WebChromeClient
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.FrameLayout
import android.widget.Toast
import android.widget.VideoView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import android.os.Handler

class MainActivity : AppCompatActivity(), SensorEventListener {

    private lateinit var sensorManager: SensorManager
    private var previousProximity = Float.MAX_VALUE
    private var isReporting = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        sensorManager = getSystemService(Context.SENSOR_SERVICE) as SensorManager
        val myWebView = findViewById<WebView>(R.id.webView)
        val mWebSettings: WebSettings = myWebView.settings

        myWebView.setOnTouchListener { _, _ -> true }

        myWebView.webViewClient = WebViewClient()
        mWebSettings.javaScriptEnabled = true
        mWebSettings.mediaPlaybackRequiresUserGesture = false

        myWebView.webViewClient = WebViewClient()
        mWebSettings.javaScriptEnabled = true
        mWebSettings.mediaPlaybackRequiresUserGesture = false
        myWebView.setLayerType(View.LAYER_TYPE_HARDWARE, null)

        myWebView.webChromeClient = object : WebChromeClient() {
            override fun onShowCustomView(view: View?, callback: CustomViewCallback?) {
                if (view is FrameLayout) {
                    val customViewContainer = view as FrameLayout?
                    if (customViewContainer?.focusedChild is VideoView) {
                        val videoView = customViewContainer.focusedChild as VideoView?
                        videoView?.setOnCompletionListener { callback?.onCustomViewHidden() }
                        videoView?.start()
                    }
                }
            }
        }

        myWebView.loadUrl("https://kcf2023.netlify.app/")

        val proximitySensor: Sensor? = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY)

        if (proximitySensor != null) {
            sensorManager.registerListener(
                this,
                proximitySensor,
                SensorManager.SENSOR_DELAY_NORMAL
            )
            if (ActivityCompat.checkSelfPermission(
                    this,
                    Manifest.permission.SEND_SMS
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                ActivityCompat.requestPermissions(
                    this,
                    arrayOf(Manifest.permission.SEND_SMS),
                    1
                )
            }
        } else {
            Toast.makeText(
                applicationContext,
                "근접 센서를 찾을 수 없습니다.",
                Toast.LENGTH_SHORT
            ).show()
        }
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}

    override fun onSensorChanged(event: SensorEvent?) {
        if (event?.sensor?.type == Sensor.TYPE_PROXIMITY) {
            val currentProximity = event.values[0]
            if (previousProximity < 5 && currentProximity >= 5 && !isReporting) {
                sendSMSMessage()
                showCancelButton()
                isReporting = true
            }
            previousProximity = currentProximity
        }
    }

    private fun sendSMSMessage() {
        val smsNumber = "+821021147032"
        val smsBody = "킨텍스 제 1관 10A홀에서 심정지 환자 발생. 신속한 출동 요망"
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.SEND_SMS
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.SEND_SMS),
                1
            )
            return
        }

        val smsManager = android.telephony.SmsManager.getDefault()
        try {
            smsManager.sendTextMessage(smsNumber, null, smsBody, null, null)
            Toast.makeText(
                applicationContext,
                "신고 메시지를 전송하였습니다.",
                Toast.LENGTH_SHORT
            ).show()
        } catch (ex: Exception) {
            Toast.makeText(
                applicationContext,
                "SMS 전송에 실패했습니다.",
                Toast.LENGTH_SHORT
            ).show()
            ex.printStackTrace()
        }
    }

    private fun showCancelButton() {
        val alertDialog = AlertDialog.Builder(this)
            .setTitle("신고 중")
            .setMessage("5초 후에 신고가 완료됩니다. 취소하시겠습니까?")
            .setPositiveButton("취소") { dialog, _ ->
                cancelReport()
                dialog.dismiss()
            }
            .setCancelable(false)
            .create()
        alertDialog.show()

        // 5초 후에 팝업이 사라지도록 설정
        Handler().postDelayed({
            alertDialog.dismiss()
        }, 5000)
    }

    private fun cancelReport() {
        val smsNumber = "+821021147032"
        val smsBody = "신고가 취소되었습니다."
        val smsManager = android.telephony.SmsManager.getDefault()
        try {
            smsManager.sendTextMessage(smsNumber, null, smsBody, null, null)
            Toast.makeText(
                applicationContext,
                "신고가 취소되었습니다.",
                Toast.LENGTH_SHORT
            ).show()
        } catch (ex: Exception) {
            Toast.makeText(
                applicationContext,
                "SMS 전송에 실패했습니다.",
                Toast.LENGTH_SHORT
            ).show()
            ex.printStackTrace()
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            1 -> {
                if ((grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    sendSMSMessage()
                } else {
                    Toast.makeText(
                        applicationContext,
                        "SMS 전송 권한이 거부되었습니다.",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            }
        }
    }

    override fun onBackPressed() {
        val myWebView = findViewById<WebView>(R.id.webView)

        if (myWebView.canGoBack()) {
            myWebView.goBack()
        } else {
            super.onBackPressed()
        }
    }
}
