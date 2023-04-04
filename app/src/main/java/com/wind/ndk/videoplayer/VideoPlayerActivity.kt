package com.wind.ndk.videoplayer

import android.os.Bundle
import android.view.SurfaceHolder
import androidx.appcompat.app.AppCompatActivity
import com.wind.ndk.videoplayer.databinding.ActivityMainBinding
import com.wind.ndk.videoplayer.databinding.ActivityVideoplayerBinding
import kotlin.concurrent.thread

/**
 * Copyright (C), 2015-2022, 杭州迈优文化创意有限公司
 * FileName: VideoPlayerActivity
 * Author: wind
 * Date: 2023/4/4 11:41
 * Description: 描述该类的作用
 * Path: 路径
 * History:
 *  <author> <time> <version> <desc>
 *
 */
class VideoPlayerActivity : AppCompatActivity() {
    private lateinit var binding: ActivityVideoplayerBinding
    private lateinit var mPlayer: VideoPlayer
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityVideoplayerBinding.inflate(layoutInflater)
        setContentView(binding.root)
        mPlayer= VideoPlayer()

        binding.surfaceView.holder.addCallback(object:SurfaceHolder.Callback{
            override fun surfaceCreated(holder: SurfaceHolder) {
                mPlayer.surfaceCreated(holder.surface)
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                mPlayer.surfaceChanged(width,height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                mPlayer.surfaceDestroyed()
            }

        })

        thread {
            mPlayer.setDataSource("")
            mPlayer.prepare()

        }


    }
}