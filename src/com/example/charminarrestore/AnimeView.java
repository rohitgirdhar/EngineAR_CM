package com.example.charminarrestore;
import java.io.InputStream;

import com.example.charminarrestore.R;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Movie;
import android.graphics.Paint;
import android.util.Log;
import android.view.View;


public class AnimeView extends View{

	InputStream is = null;
	Movie[] handGif; 
	long mMovieStart;
	private int x, y;
	float bHeight;
	int currStep;
	Context pContext;
	Bitmap temp;
	
	public AnimeView(Context context){
		super(context);
		pContext = context;
		handGif = new Movie[4];
		
    	is = context.getResources().openRawResource(R.drawable.hand);    	
    	handGif[0] = Movie.decodeStream(is);	
  
	
    	
    	mMovieStart = 0;
    	x=100;y=100;
    	currStep = 0;
	}
	
	public void setPos(float[] pose, int step){
		x = (int)pose[14];
		y = (int) pose[15];
		bHeight = pose[13];
		currStep = step;
	}
	
	public void setInvalidate(){
	    this.postInvalidate();
	}
	
    @Override
    protected void onDraw(Canvas canvas){
    	canvas.drawColor(Color.TRANSPARENT);

    	
    	
    	try{
//   	    temp = BitmapFactory.decodeFile("/sdcard/charminarAR/overlay3_1.png");
    	    canvas.drawBitmap(Sample3View.bmp_overlay, 0, 0, null);
    	}catch(Exception e){
    	    
    	}

        this.invalidate();
    }
}
