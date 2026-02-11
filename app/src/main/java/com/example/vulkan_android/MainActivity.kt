package com.example.vulkan_android

import org.libsdl.app.SDLActivity
import android.os.Bundle
import android.util.Log
import java.io.File
import java.io.FileOutputStream

class MainActivity : SDLActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        Log.d("MainActivity", "onCreate called")

        copyAssetToFile("vert.spv")
        copyAssetToFile("frag.spv")
        copyAssetToFile("comp.spv");

        super.onCreate(savedInstanceState)
        Log.d("MainActivity", "super.onCreate finished")
    }

    private fun copyAssetToFile(filename: String) {
        try {
            val outFile = File(filesDir, filename)
            Log.d("MainActivity", "Copying $filename to ${outFile.absolutePath}")

            if (!outFile.exists()) {
                assets.open(filename).use { input ->
                    FileOutputStream(outFile).use { output ->
                        input.copyTo(output)
                    }
                }
                Log.d("MainActivity", "Successfully copied $filename")
            } else {
                Log.d("MainActivity", "$filename already exists")
            }
        } catch (e: Exception) {
            Log.e("MainActivity", "Failed to copy $filename: ${e.message}")
            e.printStackTrace()
        }
    }
}