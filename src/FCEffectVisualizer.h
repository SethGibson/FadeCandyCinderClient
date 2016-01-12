/*
*
* Copyright (c) 2014, James Hurlbut
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or
* without modification, are permitted provided that the following
* conditions are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
*
* Neither the name of James Hurlbut nor the names of its
* contributors may be used to endorse or promote products
* derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#pragma once

#include "cinder/Utilities.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "FCEffect.h"

using namespace std;
using namespace ci;
using namespace ci::app;

class FCEffectVisualizer;
typedef std::shared_ptr< FCEffectVisualizer > FCEffectVisualizerRef;

struct FCVizNode {
	FCVizNode();
	FCVizNode(const FCEffect::PixelInfo &pInfo, const vec3 &pRGB);

	vec3 Pos;
	Color RGB;
};

class FCEffectVisualizer {
public:
    static FCEffectVisualizerRef create()
    {
        return ( FCEffectVisualizerRef )( new FCEffectVisualizer() );
    }
	 
	FCEffectVisualizer ();
	
    virtual void update();
	virtual void draw(vec3& rgb, const FCEffect::PixelInfo& p);
protected:
    
private:
    FCEffectRunnerRef		mEffectRunner;
	ci::JsonTree			mCurrentLayout;
	float					mScale;
	vec3					mOffset;

	vector<FCVizNode>		mVizNodes;
	gl::VboRef				mInstanceData;
	gl::BatchRef			mInstanceDraw;
	gl::GlslProgRef			mInstanceShader;
};