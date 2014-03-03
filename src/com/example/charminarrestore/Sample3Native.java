package com.example.charminarrestore;


import java.io.InputStream;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Movie;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

public class Sample3Native extends Activity implements SearchOverInterface{
    private static final String TAG = "Sample::Activity";
    private Sample3View cameraView;
	private int obj;

	private AnimeView1 drawInst;

	public Sample3Native() {
        Log.i(TAG, "Instantiated new " + this.getClass());        
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        
        int flagS;
        if(savedInstanceState == null){
			Bundle extras = getIntent().getExtras();
			if( extras == null ){
				flagS = 0;;							
			}else{
				flagS = extras.getInt("flagSource");
			}			
		}else{
			flagS = (Integer)savedInstanceState.getSerializable("flagSource");
		}  
        Log.i("Sample3Native",Integer.toString(flagS));
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        cameraView = new Sample3View(this, flagS);
        cameraView.setSearchOverInterface(this);
        cameraView.activityStopped = false;      
        
        if( cameraView.sourcePresentFlag == 0  ){
        	Toast.makeText(this, "No source files present, go to 'Select Object(s)' menu", Toast.LENGTH_LONG).show();
        	finish();        	
        }        
        
        drawInst = new AnimeView1(this);

        addContentView(cameraView, new LayoutParams(LayoutParams.MATCH_PARENT,LayoutParams.MATCH_PARENT));
        addContentView(drawInst, new LayoutParams(LayoutParams.MATCH_PARENT,LayoutParams.MATCH_PARENT));
        drawInst.bringToFront();
    }
    
	@Override
	public boolean onTouchEvent(MotionEvent event){
	    if(event.getAction() == MotionEvent.ACTION_DOWN) {
	        // Execute your Runnable after 5000 milliseconds = 5 seconds.
	        cameraView.mBooleanIsPressed = true;
	        cameraView.touchX = event.getX();
	        cameraView.touchY = event.getY();
	    }

	    if(event.getAction() == MotionEvent.ACTION_UP) {
	        if(cameraView.mBooleanIsPressed) {
	            cameraView.mBooleanIsPressed = false;

	        }
	    }
		return super.onTouchEvent(event);

	}
	
	@Override
	public void onSearchingDone(boolean success) {
		// TODO Auto-generated method stub
		Log.i("MainActivity","OnDataLoaded");
		if( success == true){
			if( drawInst.width == 0 ){
				drawInst.setCameraDims(cameraView.getFrameWidth(), cameraView.getFrameHeight());
			}
		}
	}
	
	@Override
	protected void onPause(){
		super.onPause();
		cameraView.onPause();
	}
	
	@Override
	protected void onResume(){
		super.onResume();
		cameraView.onResume();
	}
	
	public class AnimeView1 extends View{

	    Context pContext;
	    Bitmap temp;
	    int width,height;
	    
	    public AnimeView1(Context context){
	        super(context);
	        pContext = context;
	        width=0;

	    }
	    public void setCameraDims(int w, int h){
	    	width = w; height = h;
	    }
	    
	    @Override
	    protected void onDraw(Canvas canvas){
	        canvas.drawColor(Color.TRANSPARENT);
	        
	        float dx = (canvas.getWidth() - width)/2;
	        float dy = (canvas.getHeight() - height)/2;
	        canvas.translate(dx, dy);
	        Log.i("Canvas translating: ", Float.toString(dx) + " " + Float.toString(dy));
//	        canvas.scale(sx, sy);
	        Log.i("AnimeView","Drawing again");
	        super.onDraw(canvas);
	        try{
//	          temp = BitmapFactory.decodeFile("/sdcard/charminarAR/overlay3_1.png");
	            canvas.drawBitmap(Sample3View.bmp_overlay, 0, 0, null);
	        }catch(Exception e){
	            
	        }

	        this.invalidate();
	    }
	}

}
