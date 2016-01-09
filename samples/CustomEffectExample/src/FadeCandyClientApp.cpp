#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/CameraUI.h"
#include "cinder/gl/Texture.h"
#include "FCEffectRunner.h"

// -------- SPOUT -------------
#include "Spout/SpoutDLL.h"
using namespace Spout2;
// ----------------------------

using namespace ci;
using namespace ci::app;
using namespace std;

class MyEffect;
typedef std::shared_ptr< MyEffect > MyEffectRef;

class MyEffect : public FCEffect
{
public:
	static MyEffectRef create(SurfaceRef pSurf)
    {
        return MyEffectRef( new MyEffect(pSurf) );
    }
    MyEffect(SurfaceRef pSurf)
    {
		mSurf = pSurf;
	}
	float time;
	SurfaceRef mSurf;

    void beginFrame(const FrameInfo& f) override
    {
        const float speed = 1.0;
        time += f.timeDelta * speed;
    }

    void shader(vec3& rgb, const PixelInfo& p) override
    {
		if (mSurf)
		{
			vec2 pt = vec2((p.point.x), (p.point.y));
			//pt.x = pt.x/4.f;
			//pt.x += mSection;
			auto colorval = mSurf->getPixel(ivec2(pt.x*mSurf->getWidth(), pt.y*mSurf->getHeight()));
			rgb = vec3(colorval.r / 255.f, colorval.g / 255.f, colorval.b / 255.f);
		}
		else
			rgb = vec3();
	}
};

class FadeCandyClientApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
    
	void mouseMove( MouseEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void resize() override;

	bool bInitialized; // true if a sender initializes OK
	bool bDoneOnce; // only try to initialize once
	bool bTextureShare; // tells us if texture share compatible
	char SenderName[256]; // sender name 

	unsigned int g_Width, g_Height; // size of the texture being sent out
	gl::Texture2dRef spoutTexture;  // Local Cinder texture used for sharing

	FCEffectRunnerRef effectRunner;
	MyEffectRef e;
	SurfaceRef spoutSurf;

	CameraUi	mMayaCam;
	ivec2		mMousePos;
};

void FadeCandyClientApp::setup()
{
	bInitialized = false;
	g_Width = 124; // set global width and height to something
	g_Height = 8; // they need to be reset when the receiver connects to a sender
	//point FC to host and port
	effectRunner = FCEffectRunner::create("127.0.0.1",7890);
	//create instance of our custom effect
	e = MyEffect::create(spoutSurf);
	effectRunner->setEffect(e);
	effectRunner->setMaxFrameRate(400);
	effectRunner->setVerbose(true);
    effectRunner->setLayout("layouts/grid16x8.json",0);
	//add visualizer to see effect on screen
	FCEffectVisualizerRef viz = FCEffectVisualizer::create();
	effectRunner->setVisualizer(viz);
	
	// set up the camera
	CameraPersp cam;
	cam.lookAt(vec3(300.0f, 250.f, -500.0f), vec3(300.0f, 200.0f, 0.0f), vec3(0, 1, 0));
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCamera( &cam );
	gl::enableVerticalSync(false);
}

void FadeCandyClientApp::update()
{
	unsigned int width, height;
	char tempname[256];

	// -------- SPOUT -------------
	if(!bInitialized) {

		// This is a receiver, so the initialization is a little more complex than a sender

		// You can pass a sender name to try to find and connect to 
		SenderName[0] = NULL; // the name will be filled when the receiver connects to a sender
		strcpy_s(tempname, 256, "Sender name"); 
		width  = g_Width; // pass the initial width and height (they will be adjusted if necessary)
		height = g_Height;
		bInitialized = CreateReceiver(tempname, width, height, bTextureShare);
		// Initialization will fail if there are no senders, so just keep trying unti a sender starts

		if(bInitialized) {
			// Check to see whether it has initialized texture share or memoryshare
			if(bTextureShare) { 
				// Texture share is OK so we can look at sender names
				// Check if the name returned is different.
				if(strcmp(SenderName, tempname) != 0) {
					// If the sender name is different, the requested 
					// sender was not found so the active sender was used.
					// Act on this if necessary.
					strcpy_s(SenderName, 256, tempname);
				}
			}
			// else the receiver has initialized in memoryshare mode

			// Is the size of the detected sender different from the current texture size ?
			// This is detected for both texture share and memoryshare
			if(width != g_Width || height != g_Height) {

				g_Width = width;
				g_Height = height;
				// Reset the local receiving texture size
				spoutTexture =  gl::Texture2d::create(g_Width, g_Height);
				
				// reset render window
				//setWindowSize(g_Width, g_Height);
			} 


		}
		else {
			// Receiver initialization will fail if no senders are running
			// Keep trying until one starts
		}
	} // endif not initialized

	effectRunner->update();
}

void FadeCandyClientApp::draw()
{
	unsigned int width, height;
	char txt[256];

	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::viewport( getWindowSize() );
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
	gl::drawStringCentered(effectRunner->getDebugString(),vec2(getWindowCenter().x,5),Color(1,1,1),mDefault);
	gl::disableAlphaBlending();

	// Save current global width and height - they will be changed
	// by receivetexture if the sender changes dimensions
	width  = g_Width;
	height = g_Height;
	//
	// Try to receive the texture at the current size 
	//
	// NOTE :
	// if the host calls SendTexture with a framebuffer object actively bound
	// the host must provide the GL handle to its EXT_framebuffer_object
	// so that the dll can restore that binding because it makes use of its
	// own FBO for intermediate rendering - default is 0 for no bound host FBO - see Spout.h
	//
	if(bInitialized && spoutTexture) {

		if(!ReceiveTexture(SenderName, width, height, spoutTexture->getId(), spoutTexture->getTarget())) {
			//
			// Receiver failure :
			//	1)	width and height are zero for read failure.
			//	2)	width and height are changed for sender change
			//		The local texture then has to be resized.
			//
			if(width == 0 || height == 0) {
				// width and height are returned zero if there has been 
				// a texture read failure which might happen if the sender
				// is closed. Spout will keep trying and if the same sender opens again
				// will use it. Otherwise the user can select another sender.
				return;
			}

			if(width != g_Width || height != g_Height ) {
				// The sender dimensions have changed
				// Update the global width and height
				g_Width  = width;
				g_Height = height;
				// Update the local texture to receive the new dimensions
				spoutTexture =  gl::Texture2d::create(g_Width, g_Height);
				
				return; // quit for next round
			}
		}
		else {

			spoutSurf = Surface::create(spoutTexture->createSource());
			e->mSurf = spoutSurf;
		}
	}
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
	mMayaCam.setCamera( &cam );
}

void prepareSettings(App::Settings *settings)
{
	//settings->setFrameRate( 400.0f );
	settings->disableFrameRate();
}


CINDER_APP( FadeCandyClientApp, RendererGl, prepareSettings )
