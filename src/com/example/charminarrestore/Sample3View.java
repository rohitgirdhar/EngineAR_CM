package com.example.charminarrestore;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Movie;
import android.os.SystemClock;
import android.util.Log;
import android.widget.SlidingDrawer;

class Sample3View extends SampleViewBase {
	
	private int mFrameSize;
	private Bitmap mBitmap;
	private int[] mRGBA;
    private int[] mRGBA_overlay;

	public static byte mArr[];
	Context parentActivity;
	static public String res;
	static public Bitmap bmp;
	static public Bitmap bmp_overlay;
	public float touchX;
	public float touchY;
	public int currStep=0;
	public int maxSteps= 2;
	public static float[] Pose;
	public float frametime = 0;
	public int flagS;
	public int sourcePresentFlag = 1;

	
	ArrayList<SearchOverInterface> listeners = new ArrayList<SearchOverInterface>();


    public Sample3View(Context context, int flag) {
        super(context);
    	parentActivity = context;
    	res = "!";
    	flagS = flag;
    	maxSteps = LoadSource(flagS);
    	if( maxSteps == -1 ){
    		sourcePresentFlag = 0;
    	}

    }

	@Override
	protected void onPreviewStarted(int previewWidtd, int previewHeight) {
		if(mBitmap != null) {
			mBitmap.recycle();
			mBitmap = null;
		}
		mFrameSize = previewWidtd * previewHeight;
		mRGBA = new int[mFrameSize];
		mRGBA_overlay = new int[mFrameSize];
		mBitmap = Bitmap.createBitmap(previewWidtd, previewHeight, Bitmap.Config.ARGB_8888);
      bmp_overlay = Bitmap.createBitmap(previewWidtd, previewHeight, Bitmap.Config.ARGB_8888);

	}

	@Override
	protected void onPreviewStopped() {
		if(mBitmap != null) {
//			mBitmap.recycle();
//			mBitmap = null;
		}
		mRGBA = null;
	
		
		
	}

    @Override
    protected Bitmap processFrame(byte[] data) {
        int[] rgba = mRGBA;
        int[] rgba_overlay = mRGBA_overlay;
        bmp = mBitmap; 
        
       	Log.i("processFrame-search called",res);
      	    
       	double fps = 1000/(SystemClock.elapsedRealtime() - frametime);
       	frametime = SystemClock.elapsedRealtime();
       	Log.i("processFrame - fps: ", Double.toString(fps));
        int pFlag = FindFeatures(getFrameWidth(), getFrameHeight(), data, rgba, rgba_overlay, mBooleanIsPressed, touchX, touchY, currStep);
        bmp.setPixels(rgba, 0/* offset */, getFrameWidth() /* stride */, 0, 0, getFrameWidth(), getFrameHeight());
        try{
            bmp_overlay.setPixels(rgba_overlay,0/* offset */, getFrameWidth() /* stride */, 0, 0, getFrameWidth(), getFrameHeight());

        }catch(Exception e){
            
        }
        
		for( SearchOverInterface listener:listeners){
			listener.onSearchingDone(true);
		}
        
        return bmp;
    }

    
	public void setSearchOverInterface( SearchOverInterface listener){
		listeners.add(listener);
	}
    
    public native int FindFeatures(int width, int height, byte yuv[], int[] rgba, int[] rgba_overlay, boolean addRemovePt, 
    		float posX, float posY, int currStep );
    public native int LoadSource(int flagS);

    
    static {
        System.loadLibrary("opencv_java");
        System.loadLibrary("EngineAR");
    }


	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		activityStopped = true;
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		activityStopped = false;

	}
	
	

}
