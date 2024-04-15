//
// Copyright (c) 2023, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <Options.h>

#include <OptiXToolkit/ShaderUtil/vec_printers.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

namespace {

using namespace testing;

class TestOptions : public Test
{
  protected:
    demandPbrtScene::Options getOptions( std::initializer_list<const char*> args ) const;

    StrictMock<MockFunction<demandPbrtScene::UsageFn>> m_mockUsage;
    std::function<demandPbrtScene::UsageFn>            m_usage{ m_mockUsage.AsStdFunction() };
};

demandPbrtScene::Options TestOptions::getOptions( std::initializer_list<const char*> args ) const
{
    std::vector<char*> argv;
    std::transform( args.begin(), args.end(), std::back_inserter( argv ),
                    []( const char* arg ) { return const_cast<char*>( arg ); } );
    return demandPbrtScene::parseOptions( static_cast<int>( argv.size() ), argv.data(), m_usage );
}

}  // namespace

TEST_F( TestOptions, programNameParsed )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_EQ( "DemandPbrtScene", options.program );
}

TEST_F( TestOptions, missingSceneFile )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "missing scene file argument" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene" } );
}

TEST_F( TestOptions, sceneFileArgumentParsed )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_EQ( "scene.pbrt", options.sceneFile );
}

TEST_F( TestOptions, sceneFileBetweenOptions )
{
    const demandPbrtScene::Options options =
        getOptions( { "DemandPbrtScene", "--dim=128x256", "scene.pbrt", "--file", "output.png" } );

    EXPECT_EQ( 128, options.width );
    EXPECT_EQ( 256, options.height );
    EXPECT_EQ( "scene.pbrt", options.sceneFile );
    EXPECT_EQ( "output.png", options.outFile );
}

TEST_F( TestOptions, fileArgumentParsed )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "-f", "outfile.png", "scene.pbrt" } );

    EXPECT_EQ( "outfile.png", options.outFile );
}

TEST_F( TestOptions, fileArgumentMissingValue )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "missing filename argument" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "-f" } );
}

TEST_F( TestOptions, longFormFileArgument )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--file", "outfile.png", "scene.pbrt" } );

    EXPECT_EQ( "outfile.png", options.outFile );
}

TEST_F( TestOptions, dimensionsDefaultTo768x512 )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_EQ( 768, options.width );
    EXPECT_EQ( 512, options.height );
}

TEST_F( TestOptions, dimensionsParsed )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--dim=256x512", "scene.pbrt" } );

    EXPECT_EQ( 256, options.width );
    EXPECT_EQ( 512, options.height );
}

TEST_F( TestOptions, defaultBackgroundIsBlack )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_EQ( make_float3( 0.0f, 0.0f, 0.0f ), options.background );
}

TEST_F( TestOptions, parseBackgroundColor )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--bg=0.1/0.2/0.3", "scene.pbrt" } );

    EXPECT_EQ( make_float3( 0.1f, 0.2f, 0.3f ), options.background );
}

TEST_F( TestOptions, oneShotGeometry )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--oneshot-geometry", "scene.pbrt" } );

    EXPECT_TRUE( options.oneShotGeometry );
}

TEST_F( TestOptions, oneShotMaterial )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--oneshot-material", "scene.pbrt" } );

    EXPECT_TRUE( options.oneShotMaterial );
}

TEST_F( TestOptions, noProxyResolutionLoggingByDefault )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_FALSE( options.verboseProxyGeometryResolution );
    EXPECT_FALSE( options.verboseProxyMaterialResolution );
}

TEST_F( TestOptions, verboseProxyResolution )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--proxy-resolution", "scene.pbrt" } );

    EXPECT_TRUE( options.verboseProxyGeometryResolution );
    EXPECT_TRUE( options.verboseProxyMaterialResolution );
}

TEST_F( TestOptions, verboseProxyGeometryResolution )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--proxy-geometry", "scene.pbrt" } );

    EXPECT_TRUE( options.verboseProxyGeometryResolution );
    EXPECT_FALSE( options.verboseProxyMaterialResolution );
}

TEST_F( TestOptions, verboseProxyMaterialResolution )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--proxy-material", "scene.pbrt" } );

    EXPECT_FALSE( options.verboseProxyGeometryResolution );
    EXPECT_TRUE( options.verboseProxyMaterialResolution );
}

TEST_F( TestOptions, noSceneDecompositionByDefault )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_FALSE( options.verboseSceneDecomposition );
}

TEST_F( TestOptions, verboseSceneDecomposition )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--scene-decomposition", "scene.pbrt" } );

    EXPECT_TRUE( options.verboseSceneDecomposition );
}

TEST_F( TestOptions, noTextureCreationByDefault )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_FALSE( options.verboseTextureCreation );
}

TEST_F( TestOptions, verboseTextureCreation )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--texture-creation", "scene.pbrt" } );

    EXPECT_TRUE( options.verboseTextureCreation );
}

TEST_F( TestOptions, verboseLogging )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--verbose", "scene.pbrt" } );

    EXPECT_TRUE( options.verboseProxyGeometryResolution );
    EXPECT_TRUE( options.verboseProxyMaterialResolution );
    EXPECT_TRUE( options.verboseSceneDecomposition );
    EXPECT_TRUE( options.verboseTextureCreation );
}

TEST_F( TestOptions, proxiesNotSortedByDefault )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "scene.pbrt" } );

    EXPECT_FALSE( options.sortProxies );
}

TEST_F( TestOptions, sortProxies )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--sort-proxies", "scene.pbrt" } );

    EXPECT_TRUE( options.sortProxies );
}

TEST_F( TestOptions, sync )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--sync", "scene.pbrt" } );

    EXPECT_TRUE( options.sync );
}

TEST_F( TestOptions, faceForward )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--face-forward", "scene.pbrt" } );

    EXPECT_TRUE( options.faceForward );
}

TEST_F( TestOptions, backgroundMissing3rdValue )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad background color value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--bg=0.1/0.2", "scene.pbrt" } );
}

TEST_F( TestOptions, backgroundWithNegativeRedValue )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad background color value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--bg=-1/2/3", "scene.pbrt" } );
}

TEST_F( TestOptions, backgroundWithNegativeGreenValue )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad background color value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--bg=1/-2/3", "scene.pbrt" } );
}

TEST_F( TestOptions, backgroundWithNegativeBlueValue )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad background color value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--bg=1/2/-3", "scene.pbrt" } );
}

TEST_F( TestOptions, warmupFrameCount )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--warmup=10", "scene.pbrt" } );

    EXPECT_EQ( 10, options.warmupFrames );
}

TEST_F( TestOptions, negativeWarmupCountInvalid )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad warmup frame count value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--warmup=-10", "scene.pbrt" } );
}

TEST_F( TestOptions, missingWarmupCountInvalid )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad warmup frame count value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--warmup=", "scene.pbrt" } );
}

TEST_F( TestOptions, parseDebugPixel )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=384/256", "scene.pbrt" } );

    EXPECT_TRUE( options.debug );
    EXPECT_EQ( make_int2( 384, 256 ), options.debugPixel );
}

TEST_F( TestOptions, oneShotDebug )
{
    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--oneshot-debug", "scene.pbrt" } );

    EXPECT_TRUE( options.oneShotDebug );
}

TEST_F( TestOptions, negativeDebugPixelX )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=-1/256", "scene.pbrt" } );
}

TEST_F( TestOptions, negativeDebugPixelY )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=384/-1", "scene.pbrt" } );
}

TEST_F( TestOptions, missingDebugPixelY )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=384/", "scene.pbrt" } );
}

TEST_F( TestOptions, missingDebugPixelSeparator )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=384", "scene.pbrt" } );
}

TEST_F( TestOptions, missingDebugPixelX )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=", "scene.pbrt" } );
}

TEST_F( TestOptions, tooLargeDebugPixelX )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=384/128", "--dim=256x256", "scene.pbrt" } );
}

TEST_F( TestOptions, tooLargeDebugPixelY )
{
    EXPECT_CALL( m_mockUsage, Call( StrEq( "DemandPbrtScene" ), StrEq( "bad debug pixel value" ) ) ).Times( 1 );

    const demandPbrtScene::Options options = getOptions( { "DemandPbrtScene", "--debug=128/384", "--dim=256x256", "scene.pbrt" } );
}
