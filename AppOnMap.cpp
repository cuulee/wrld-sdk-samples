//
//  AppOnMap.cpp
//  ExampleApp
//

#include "AppOnMap.h"
#include "EegeoWorld.h"
#include "GlobeCameraTouchController.h"
#include "RenderCamera.h"
#include "GlobeCameraController.h"
#include "GlobeCameraInterestPointProvider.h"
#include "GlobeCameraController.h"
#include "CameraHelpers.h"
#include "WeatherController.h"
#include "NativeUIFactories.h"

#include "DebugSphereExample.h"
#include "ScreenUnprojectExample.h"
#include "LoadModelExample.h"
#include "EnvironmentNotifierExample.h"
#include "WebRequestExample.h"
#include "FileIOExample.h"
#include "NavigationGraphExample.h"
#include "ModifiedRenderingExample.h"
#include "ToggleTrafficExample.h"
#include "ResourceSpatialQueryExample.h"
#include "EnvironmentFlatteningExample.h"
#include "SearchExample.h"
#include "KeyboardInputExample.h"
#include "PODAnimationExample.h"
#include "Pick3DObjectExample.h"
#include "ScreenPickExample.h"
#include "DebugPrimitiveRenderingExample.h"

MyApp::MyApp(Eegeo::Camera::GlobeCamera::GlobeCameraInterestPointProvider& globeCameraInterestPointProvider,
             ExampleTypes::Examples selectedExample)
: m_globeCameraController(NULL)
, m_cameraTouchController(NULL)
, m_globeCameraInterestPointProvider(globeCameraInterestPointProvider)
, m_selectedExampleType(selectedExample)
, pExample(NULL)
{
    Eegeo_ASSERT(&m_globeCameraInterestPointProvider != NULL);
}

MyApp::~MyApp()
{
    pExample->Suspend();
    delete pExample;
    delete m_globeCameraController;
    delete m_cameraTouchController;
}

void MyApp::OnStart ()
{
    Eegeo_ASSERT(&World() != NULL);
    
    Eegeo::EegeoWorld& eegeoWorld = World();
    
    
    Eegeo::Camera::GlobeCamera::GlobeCameraTouchControllerConfiguration touchControllerConfig = Eegeo::Camera::GlobeCamera::GlobeCameraTouchControllerConfiguration::CreateDefault();
    
    // override default configuration to enable two-finger pan gesture to control additional camera pitch
    touchControllerConfig.tiltEnabled = true;
    
    m_cameraTouchController = new Eegeo::Camera::GlobeCamera::GlobeCameraTouchController(touchControllerConfig);
    
    const bool useLowSpecSettings = false;
    Eegeo::Camera::GlobeCamera::GlobeCameraControllerConfiguration globeCameraControllerConfig = Eegeo::Camera::GlobeCamera::GlobeCameraControllerConfiguration::CreateDefault(useLowSpecSettings);

    m_globeCameraController = new Eegeo::Camera::GlobeCamera::GlobeCameraController(eegeoWorld.GetTerrainHeightProvider(),
                                                                                    eegeoWorld.GetEnvironmentFlatteningService(),
                                                                                    eegeoWorld.GetResourceCeilingProvider(),
                                                                                    *m_cameraTouchController,
                                                                                    globeCameraControllerConfig);
    

    
    Eegeo::Camera::RenderCamera* renderCamera = m_globeCameraController->GetCamera();
    const Eegeo::Rendering::RenderContext& renderContext = eegeoWorld.GetRenderContext();
    renderCamera->SetViewport(0.f, 0.f, renderContext.GetScreenWidth(), renderContext.GetScreenHeight());
    eegeoWorld.SetCamera(renderCamera);
    
    m_globeCameraInterestPointProvider.SetGlobeCamera(m_globeCameraController);

    float interestPointLatitudeDegrees = 37.7858f;
    float interestPointLongitudeDegrees = -122.401f;
    float interestPointAltitudeMeters = 2.7;
    
    Eegeo::Space::LatLongAltitude location = Eegeo::Space::LatLongAltitude::FromDegrees(interestPointLatitudeDegrees,
                                                                                        interestPointLongitudeDegrees,
                                                                                        interestPointAltitudeMeters);
    
    
    
    float cameraControllerOrientationDegrees = 0.0f;
    float cameraControllerDistanceFromInterestPointMeters = 1781.0f;
    
    Eegeo::Space::EcefTangentBasis cameraInterestBasis;
    Eegeo::Camera::CameraHelpers::EcefTangentBasisFromPointAndHeading(location.ToECEF(), cameraControllerOrientationDegrees, cameraInterestBasis);
    
    m_globeCameraController->SetView(cameraInterestBasis, cameraControllerDistanceFromInterestPointMeters);
    
    eegeoWorld.GetWeatherController().SetWeather(Eegeo::Weather::Sunny, 1.0f);
    
    Eegeo::Search::Service::SearchService* searchService = NULL;
    
    if (eegeoWorld.IsSearchServiceAvailable())
    {
        searchService = &eegeoWorld.GetSearchService();
    }
    
    pExample = CreateExample(m_selectedExampleType,
                             eegeoWorld.GetRenderContext(),
                             location,
                             eegeoWorld.GetCameraProvider(),
                             *m_globeCameraController,
                             *m_cameraTouchController,
                             eegeoWorld.GetTerrainHeightProvider(),
                             eegeoWorld.GetTextureLoader(),
                             eegeoWorld.GetFileIO(),
                             eegeoWorld.GetTerrainStreaming(),
                             eegeoWorld.GetWebRequestFactory(),
                             eegeoWorld.GetNavigationGraphRepository(),
                             eegeoWorld.GetBuildingMeshPool(),
                             eegeoWorld.GetShadowMeshPool(),
                             eegeoWorld.GetStreamingVolume(),
                             eegeoWorld.GetGlobalLighting(),
                             eegeoWorld.GetGlobalFogging(),
                             eegeoWorld.GetTrafficSimulation(),
                             eegeoWorld.GetResourceSpatialQueryService(),
                             eegeoWorld.GetEnvironmentFlatteningService(),
                             searchService,
                             eegeoWorld.GetNativeUIFactories(),
                             eegeoWorld.GetInterestPointProvider());
    
    pExample->Start();
}

void MyApp::Update (float dt)
{
    Eegeo::EegeoWorld& eegeoWorld = World();
    
    eegeoWorld.EarlyUpdate(dt);
    
    m_cameraTouchController->Update(dt);
    m_globeCameraController->Update(dt);
    
    eegeoWorld.Update(dt);
    pExample->Update();
}

void MyApp::Draw (float dt)
{
    Eegeo::Rendering::GLState& glState = World().GetRenderContext().GetGLState();
    glState.ClearColor(0.8f, 0.8f, 0.8f, 1.f);
    World().Draw(dt);
    pExample->Draw();
}

void MyApp::JumpTo(double latitudeDegrees, double longitudeDegrees, double altitudeMetres, double headingDegrees, double distanceToInterestMetres)
{
    Eegeo::dv3 interestPoint = Eegeo::Space::LatLongAltitude::FromDegrees(latitudeDegrees, longitudeDegrees, altitudeMetres).ToECEF();
    
    Eegeo::Space::EcefTangentBasis interestBasis;
    
    Eegeo::Camera::CameraHelpers::EcefTangentBasisFromPointAndHeading(interestPoint, headingDegrees, interestBasis);
    
    m_globeCameraController->SetView(interestBasis, distanceToInterestMetres);
}

Examples::IExample* MyApp::CreateExample(ExampleTypes::Examples example,
                                         Eegeo::Rendering::RenderContext& renderContext,
                                         Eegeo::Space::LatLongAltitude interestLocation,
                                         Eegeo::Camera::ICameraProvider& cameraProvider,
                                         Eegeo::Camera::GlobeCamera::GlobeCameraController& globeCameraController,
                                         Eegeo::ITouchController& cameraTouchController,
                                         Eegeo::Resources::Terrain::Heights::TerrainHeightProvider& terrainHeightProvider,
                                         Eegeo::Helpers::ITextureFileLoader& textureLoader,
                                         Eegeo::Helpers::IFileIO& fileIO,
                                         Eegeo::Resources::Terrain::TerrainStreaming& terrainStreaming,
                                         Eegeo::Web::IWebLoadRequestFactory& webRequestFactory,
                                         Eegeo::Resources::Roads::Navigation::NavigationGraphRepository& navigationGraphs,
                                         Eegeo::Resources::MeshPool<Eegeo::Rendering::RenderableItem*>& buildingPool,
                                         Eegeo::Resources::MeshPool<Eegeo::Rendering::RenderableItem*>& shadowPool,
                                         Eegeo::Streaming::IStreamingVolume& visibleVolume,
                                         Eegeo::Lighting::GlobalLighting& lighting,
                                         Eegeo::Lighting::GlobalFogging& fogging,
                                         Eegeo::Traffic::TrafficSimulation& trafficSimulation,
                                         Eegeo::Resources::ResourceSpatialQueryService& resourceSpatialQueryService,
                                         Eegeo::Rendering::EnvironmentFlatteningService& environmentFlatteningService,
                                         Eegeo::Search::Service::SearchService* searchService,
                                         Eegeo::UI::NativeUIFactories& nativeInputFactories,
                                         Eegeo::Location::IInterestPointProvider& interestPointProvider)
{
    switch(example)
    {
        case ExampleTypes::LoadModel:
            return new Examples::LoadModelExample(renderContext,
                                                  interestLocation,
                                                  fileIO,
                                                  textureLoader,
                                                  fogging);
        case ExampleTypes::ScreenUnproject:
        case ExampleTypes::TerrainHeightQuery:
            return new Examples::ScreenUnprojectExample(renderContext,
                                                        cameraProvider,
                                                        terrainHeightProvider);
            
        case ExampleTypes::ScreenPick:
            return new Examples::ScreenPickExample(renderContext,
                                                   cameraProvider,
                                                   terrainHeightProvider);

        case ExampleTypes::DebugSphere:
            return new Examples::DebugSphereExample(renderContext,
                                                    interestLocation);
        case ExampleTypes::EnvironmentNotifier:
            return new Examples::EnvironmentNotifierExample(renderContext,
                                                            terrainStreaming);
        case ExampleTypes::WebRequest:
            return new Examples::WebRequestExample(webRequestFactory);
            
        case ExampleTypes::FileIO:
            return new Examples::FileIOExample(fileIO);
            
        case ExampleTypes::NavigationGraph:
            return new Examples::NavigationGraphExample(renderContext,
                                                        navigationGraphs);
            
        case ExampleTypes::ModifiedRendering:
            return new Examples::ModifiedRenderingExample(renderContext,
                                                          cameraProvider,
                                                          interestPointProvider,
                                                          visibleVolume,
                                                          lighting,
                                                          buildingPool,
                                                          shadowPool);
            
        case ExampleTypes::ToggleTraffic:
            return new Examples::ToggleTrafficExample(trafficSimulation);
            
        case ExampleTypes::ResourceSpatialQuery:
            return new Examples::ResourceSpatialQueryExample(resourceSpatialQueryService,
                                                             interestPointProvider);
            
        case ExampleTypes::EnvironmentFlattening:
            return new Examples::EnvironmentFlatteningExample(environmentFlatteningService);
            
        case ExampleTypes::Search:
            Eegeo_ASSERT(searchService != NULL, "Cannot run Search example, you must set up here.com Credentials in ViewController.mm");
            return new Examples::SearchExample(*searchService, interestPointProvider);
            
        case ExampleTypes::KeyboardInput:
            return new Examples::KeyboardInputExample(nativeInputFactories.IKeyboardInputFactory());
            
        case ExampleTypes::PODAnimation:
            return new Examples::PODAnimationExample(renderContext,
                                                     fileIO,
                                                     textureLoader,
                                                     fogging);
        case ExampleTypes::Pick3DObject:
            return new Examples::Pick3DObjectExample(renderContext,
                                                     interestLocation,
                                                     cameraProvider);
        case ExampleTypes::DebugPrimitiveRendering:
            return new Examples::DebugPrimitiveRenderingExample(World().GetDebugPrimitiveRenderer());
            
            
    }
}


void MyApp::Event_TouchRotate(const AppInterface::RotateData& data)
{
    if(!pExample->Event_TouchRotate(data))
    {
        m_cameraTouchController->Event_TouchRotate(data);
    }
}

void MyApp::Event_TouchRotate_Start(const AppInterface::RotateData& data)
{
    if(!pExample->Event_TouchRotate_Start(data))
    {
        m_cameraTouchController->Event_TouchRotate_Start(data);
    }
}

void MyApp::Event_TouchRotate_End(const AppInterface::RotateData& data)
{
    if(!pExample->Event_TouchRotate_End(data))
    {
        m_cameraTouchController->Event_TouchRotate_End(data);
    }
}

void MyApp::Event_TouchPinch(const AppInterface::PinchData& data)
{
    if(!pExample->Event_TouchPinch(data))
    {
        m_cameraTouchController->Event_TouchPinch(data);
    }
}

void MyApp::Event_TouchPinch_Start(const AppInterface::PinchData& data)
{
    if(!pExample->Event_TouchPinch_Start(data))
    {
        m_cameraTouchController->Event_TouchPinch_Start(data);
    }
}

void MyApp::Event_TouchPinch_End(const AppInterface::PinchData& data)
{
    if(!pExample->Event_TouchPinch_End(data))
    {
        m_cameraTouchController->Event_TouchPinch_End(data);
    }
}

void MyApp::Event_TouchPan(const AppInterface::PanData& data)
{
    if(!pExample->Event_TouchPan(data))
    {
        m_cameraTouchController->Event_TouchPan(data);
    }
}

void MyApp::Event_TouchPan_Start(const AppInterface::PanData& data)
{
    if(!pExample->Event_TouchPan_Start(data))
    {
        m_cameraTouchController->Event_TouchPan_Start(data);
    }
}

void MyApp::Event_TouchPan_End(const AppInterface::PanData& data)
{
    if(!pExample->Event_TouchPan_End(data))
    {
        m_cameraTouchController->Event_TouchPan_End(data);
    }
}

void MyApp::Event_TouchTap(const AppInterface::TapData& data)
{
    if(!pExample->Event_TouchTap(data))
    {
        m_cameraTouchController->Event_TouchTap(data);
    }
}

void MyApp::Event_TouchDoubleTap(const AppInterface::TapData& data)
{
    if(!pExample->Event_TouchDoubleTap(data))
    {
        m_cameraTouchController->Event_TouchDoubleTap(data);
    }
}

void MyApp::Event_TouchDown(const AppInterface::TouchData& data)
{
    if(!pExample->Event_TouchDown(data))
    {
        m_cameraTouchController->Event_TouchDown(data);
    }
}

void MyApp::Event_TouchMove(const AppInterface::TouchData& data)
{
    if(!pExample->Event_TouchMove(data))
    {
        m_cameraTouchController->Event_TouchMove(data);
    }
}

void MyApp::Event_TouchUp(const AppInterface::TouchData& data)
{
    if(!pExample->Event_TouchUp(data))
    {
        m_cameraTouchController->Event_TouchUp(data);
    }
}

