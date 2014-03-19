#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "FCEffectRunner.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"

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
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	FCEffectRunnerRef effectRunner;
};

void FadeCandyClientApp::setup()
{
	effectRunner = FCEffectRunner::create("localhost",7890);
	MyEffectRef e = MyEffect::create();
	effectRunner->setEffect(boost::dynamic_pointer_cast<FCEffect>( e ));
	effectRunner->setMaxFrameRate(100);
    effectRunner->setLayout("layouts/strip64.json");

}

void FadeCandyClientApp::mouseDown( MouseEvent event )
{
}

void FadeCandyClientApp::update()
{
	effectRunner->update();
}

void FadeCandyClientApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( FadeCandyClientApp, RendererGl )