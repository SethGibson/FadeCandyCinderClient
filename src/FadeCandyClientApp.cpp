#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "FCEffectRunner.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MyEffect;
typedef boost::shared_ptr< MyEffect > MyEffectRef;

class MyEffect : public FCEffect
{
public:
	static MyEffectRef create()
    {
        return ( MyEffectRef )( new MyEffect() );
    }
    MyEffect()
        : cycle (0) {
		mPerlin = Perlin();
	}

    float cycle;
	Perlin					mPerlin;

    void beginFrame(const FrameInfo& f)
    {
        const float speed = 10.0;
        cycle = fmodf(cycle + f.timeDelta * speed, 2 * M_PI);
    }

    void shader(ci::Vec3f& rgb, const PixelInfo& p)
    {
        float distance = p.point.length();
        float wave = sinf(3.0 * distance - cycle) + mPerlin.fBm(p.point);
		rgb = rgbToHSV(Color(0.2, 0.3, wave));
    }
};

class FadeCandyClientApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
	FCEffectRunnerRef effectRunner; 
	MayaCamUI	mMayaCam;
	// keep track of the mouse
	Vec2i		mMousePos;

	void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void resize();
};

void FadeCandyClientApp::setup()
{
	
	//point FC to host and port
	effectRunner = FCEffectRunner::create("localhost",7890);
	//create instance of our custom effect
	MyEffectRef e = MyEffect::create();
	effectRunner->setEffect(boost::dynamic_pointer_cast<FCEffect>( e ));
	effectRunner->setMaxFrameRate(100);
	effectRunner->setVerbose(true);
    effectRunner->setLayout("layouts/strip64.json");
	//add visualizer to see effect on screen
	FCEffectVisualizerRef viz = FCEffectVisualizer::create();
	effectRunner->setVisualizer(viz);
	
	// set up the camera
	CameraPersp cam;
	cam.setEyePoint( Vec3f(0.0f, -10.0f, 0.0f) );
	cam.setCenterOfInterestPoint( Vec3f(100.0f, 50.0f, 0.0f) );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCurrentCam( cam );
	
}

void FadeCandyClientApp::update()
{
	effectRunner->update();
}

void FadeCandyClientApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::setViewport( getWindowBounds() );
	gl::color( Color(1,1,1) );
	gl::setMatrices( mMayaCam.getCamera() );
	effectRunner->draw();

	//draw debug info
	gl::setMatricesWindow( getWindowSize() );
	
	Font mDefault;
	#if defined( CINDER_COCOA )        
				mDefault = Font( "Helvetica", 16 );
	#elif defined( CINDER_MSW )    
				mDefault = Font( "Arial", 16 );
	#endif
	gl::enableAlphaBlending();
	gl::drawStringCentered(effectRunner->getDebugString(),Vec2f(getWindowCenter().x,5),Color(1,1,1),mDefault);
	gl::disableAlphaBlending();
}
//camera interaction
void FadeCandyClientApp::mouseMove( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();
}

void FadeCandyClientApp::mouseDown( MouseEvent event )
{	
	// let the camera handle the interaction
	mMayaCam.mouseDown( event.getPos() );
}

void FadeCandyClientApp::mouseDrag( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();

	// let the camera handle the interaction
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void FadeCandyClientApp::resize()
{
	// adjust aspect ratio
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

CINDER_APP_NATIVE( FadeCandyClientApp, RendererGl )
