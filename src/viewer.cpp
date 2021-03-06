#ifdef USE_VIEWER
#include "cfsd/viewer.hpp"

namespace cfsd {

Viewer::Viewer() {
    pointSize = Config::get<float>("pointSize");
    landmarkSize = Config::get<float>("landmarkSize");
    lineWidth = Config::get<float>("lineWidth");
    cameraSize = Config::get<float>("cameraSize");
    cameraLineWidth = Config::get<float>("cameraLineWidth");
    viewpointX = Config::get<float>("viewpointX");
    viewpointY = Config::get<float>("viewpointY");
    viewpointZ = Config::get<float>("viewpointZ");
    viewpointF = Config::get<float>("viewpointF");
    axisDirection = Config::get<int>("axisDirection");
    background = Config::get<int>("background");
}

void Viewer::run() {
    // Create OpenGL window in single line
    pangolin::CreateWindowAndBind("Viewer", 1024, 768);

    // 3D Mouse handler requires depth testing to be enabled
    glEnable(GL_DEPTH_TEST);

    // Issue sepcific OpenGL might be needed
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int PANEL_WIDTH = 220;

    // Add named Panel and bind to variables beginning 'menu'
    // A Panel is just a View with a default layout and input handling
    pangolin::CreatePanel("menu").SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(PANEL_WIDTH));

    // Safe and efficient binding of named variables
    // Specialisations mean no conversions take place for exact types and conversions between scalar types are cheap
    pangolin::Var<bool> menuSaveWindow("menu.Save Window", false, false);
    pangolin::Var<bool> menuSaveObject("menu.Save Object", false, false);
    pangolin::Var<bool> menuFollowBody("menu.Follow Body", true, true);
    pangolin::Var<bool> menuShowCoordinate("menu.Show Coordinate", true, true);
    pangolin::Var<bool> menuShowRawPosition("menu.Show Raw Position", false, true);
    pangolin::Var<bool> menuShowPosition("menu.Show Position", true, true);
    pangolin::Var<bool> menuShowPose("menu.Show Pose", true, true);
    pangolin::Var<bool> menuShowLandmark("menu.Show Landmark", true, true);
    pangolin::Var<bool> menuShowLoopConnection("menu.Show Loop Connection", true, true);
    pangolin::Var<bool> menuShowFullBAPosition("menu.Show Full BA Position", false, true);
    // ...
    pangolin::Var<bool> menuReset("menu.Reset", false, false);
    pangolin::Var<bool> menuExit("menu.Exit", false, false);

    // Define Camera Render Object (for view / sceno browsing)
    pangolin::OpenGlRenderState s_cam;
    switch (axisDirection) {
        case 0:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisNone)
            );
            break;
        case 1:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisNegX)
            );
            break;
        case 2:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisX)
            );
            break;
        case 3:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisNegY)
            );
            break;
        case 4:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisY)
            );
            break;
        case 5:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisNegZ)
            );
            break;
        case 6:
            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(1024,768, viewpointF,viewpointF, 512,389, 0.1,1000),
                pangolin::ModelViewLookAt(viewpointX,viewpointY,viewpointZ, 0,0,0, pangolin::AxisZ)
            );
    }

    // Add named OpenGL viewport to window and provide 3D Handler
    pangolin::View& d_cam = pangolin::CreateDisplay()
        .SetBounds(0.0, 1.0, pangolin::Attach::Pix(PANEL_WIDTH), 1.0, -1024.0f/768.0f)
        .SetHandler(new pangolin::Handler3D(s_cam));

    T_WB.SetIdentity();

    // Defalut hooks for existing (Esc) and fullscreen (tab)
    while (!pangolin::ShouldQuit()) {
        // Clear entire screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Make the background white (default black)
        if (background) glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // Activate efficiently by object
        d_cam.Activate(s_cam);

        if (pangolin::Pushed(menuSaveWindow))
            pangolin::SaveWindowOnRender("window");

        if (pangolin::Pushed(menuSaveObject))
            d_cam.SaveOnRender("object");

        if (menuFollowBody)
            followBody(s_cam);

        if (menuShowCoordinate)
            drawCoordinate();

        if (menuShowRawPosition)
            drawRawPosition();

        if (menuShowPosition)
            drawPosition();

        if (menuShowPose)
            drawPose(T_WB);

        if (menuShowLandmark)
            drawLandmark();

        if (menuShowLoopConnection)
            drawLoopConnection();

        if (menuShowFullBAPosition)
            drawFullBAPosition();
        
        if (pangolin::Pushed(menuReset)) {
            std::lock_guard<std::mutex> lockPosition(positionMutex);
            std::lock_guard<std::mutex> lockRawPosition(rawPositionMutex);
            positions.clear();
            rawPositions.clear();
            readyToDrawPosition = false; readyToDrawRawPosition = false;
        }

        // Swap frames and Precess Events
        pangolin::FinishFrame();

        if (pangolin::Pushed(menuExit)) break;
    }
}

void Viewer::drawCoordinate() {
    float len = 1.0f;
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(len, 0.0f, 0.0f);

    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, len, 0.0f);
    
    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, len);
    glEnd();
}

void Viewer::genOpenGlMatrix(const Eigen::Matrix3f& R, const Eigen::Vector3d& p, pangolin::OpenGlMatrix& M) {
    M.m[0] = R(0,0);
    M.m[1] = R(1,0);
    M.m[2] = R(2,0);
    M.m[3] = 0.0;
    M.m[4] = R(0,1);
    M.m[5] = R(1,1);
    M.m[6] = R(2,1);
    M.m[7] = 0.0;
    M.m[8] = R(0,2);
    M.m[9] = R(1,2);
    M.m[10] = R(2,2);
    M.m[11] = 0.0;
    M.m[12] = p.x();
    M.m[13] = p.y();
    M.m[14] = p.z();
    M.m[15] = 1.0;
}

void Viewer::followBody(pangolin::OpenGlRenderState& s_cam) {
    if (readyToDrawPose && readyToDrawPosition) {
        std::lock_guard<std::mutex> lockPose(poseMutex);
        std::lock_guard<std::mutex> lockPosition(positionMutex);
        genOpenGlMatrix(pose, positions.back(), T_WB);
    }
    s_cam.Follow(T_WB);
}

void Viewer::pushRawPosition(const Eigen::Vector3d& p, const int& offset) {
    std::lock_guard<std::mutex> lockRawPosition(rawPositionMutex);

    unsigned i = idx + offset;
    if (rawPositions.size() <= i)
        rawPositions.push_back(p);
    else
        rawPositions[i] = p;

    readyToDrawRawPosition = true;
}

void Viewer::pushPosition(const Eigen::Vector3d& p, const int& offset) {
    std::lock_guard<std::mutex> lockPosition(positionMutex);

    unsigned i = idx + offset;
    if (positions.size() <= i)
        positions.push_back(p);
    else
        positions[i] = p;
    if (positions.size() >= WINDOWSIZE && offset == WINDOWSIZE - 1) idx++;

    readyToDrawPosition = true;
}

void Viewer::pushPose(const Eigen::Matrix3d& R) {
    std::lock_guard<std::mutex> lockPose(poseMutex);
    
    pose = R.cast<float>();

    readyToDrawPose = true;
}

void Viewer::pushLandmark(const std::map<size_t, cfsd::Ptr<MapPoint>>& pMPs) {
    std::lock_guard<std::mutex> lockLandmark(landmarkMutex);

    pMapPoints = pMPs;

    readyToDrawLandmark = true;
}

void Viewer::pushLoopConnection(const int& refFrameID, const int& curFrameID) {
    std::lock_guard<std::mutex> lockLoop(loopMutex);

    loopConnection.push_back(std::make_pair(refFrameID, curFrameID));

    readyToDrawLoop = true;
}

void Viewer::pushFullBAPosition(const Eigen::Vector3d& p) {
    std::lock_guard<std::mutex> lockPosition(fullBAPositionMutex);
    fullBAPositions.push_back(p);

    readyToDrawFullBAPosition = true;
}

void Viewer::drawRawPosition() {
    std::lock_guard<std::mutex> lockRawPosition(rawPositionMutex);

    if (!readyToDrawRawPosition) return;

    glColor3f(0.6f, 0.2f, 0.2f);
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    for (unsigned i = 0; i < rawPositions.size(); i++)
        glVertex3f(rawPositions[i].x(), rawPositions[i].y(), rawPositions[i].z());
    glEnd();

    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex3f(rawPositions[0].x(), rawPositions[0].y(), rawPositions[0].z());
    for (unsigned i = 0; i < rawPositions.size(); i++) {
        glVertex3f(rawPositions[i].x(), rawPositions[i].y(), rawPositions[i].z());
        glVertex3f(rawPositions[i].x(), rawPositions[i].y(), rawPositions[i].z());
    }
    glVertex3f(rawPositions.back().x(), rawPositions.back().y(), rawPositions.back().z());
    glEnd();
}

void Viewer::drawPosition() {
    std::lock_guard<std::mutex> lockPosition(positionMutex);

    if (!readyToDrawPosition) return;

    int n = positions.size()-WINDOWSIZE;
    if (n < 0) n = positions.size();

    glPointSize(pointSize);
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_POINTS);
    for (int i = 0; i < n; i++)
        glVertex3f(positions[i].x(), positions[i].y(), positions[i].z());
    glEnd();

    glPointSize(pointSize+4);
    glColor3f(0.8f, 0.1f, 0.1f);
    glBegin(GL_POINTS);
    for (unsigned i = n; i < positions.size(); i++)
        glVertex3f(positions[i].x(), positions[i].y(), positions[i].z());
    glEnd();

    glLineWidth(lineWidth);
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(positions[0].x(), positions[0].y(), positions[0].z());
    for (unsigned i = 1; i < positions.size() - 1; i++) {
        glVertex3f(positions[i].x(), positions[i].y(), positions[i].z());
        glVertex3f(positions[i].x(), positions[i].y(), positions[i].z());
    }
    glVertex3f(positions.back().x(), positions.back().y(), positions.back().z());
    glEnd();
}

void Viewer::drawPose(pangolin::OpenGlMatrix &M) {
    std::lock_guard<std::mutex> lockPose(poseMutex);

    if (!readyToDrawPose) return;

    const float &w = cameraSize;
    const float h = w*0.75f;
    const float z = w*0.5f;

    glPushMatrix();

    #ifdef HAVE_GLES
    glMultMatrixf(M.m);
    #else
    glMultMatrixd(M.m);
    #endif

    glLineWidth(cameraLineWidth);
    glColor3f(0.6f,0.2f,0.2f);
    glBegin(GL_LINES);

    #ifdef CFSD
    glVertex3f(0,0,0);
    glVertex3f(z,w,h);
    glVertex3f(0,0,0);
    glVertex3f(z,w,-h);
    glVertex3f(0,0,0);
    glVertex3f(z,-w,-h);
    glVertex3f(0,0,0);
    glVertex3f(z,-w,h);
    glVertex3f(z,w,h);
    glVertex3f(z,w,-h);
    glVertex3f(z,-w,h);
    glVertex3f(z,-w,-h);
    glVertex3f(z,-w,h);
    glVertex3f(z,w,h);
    glVertex3f(z,-w,-h);
    glVertex3f(z,w,-h);
    #else
    glVertex3f(0,0,0);
    glVertex3f(w,h,z);
    glVertex3f(0,0,0);
    glVertex3f(w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);
    glVertex3f(w,-h,z);
    glVertex3f(-w,h,z);
    glVertex3f(-w,-h,z);
    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);
    glVertex3f(-w,-h,z);
    glVertex3f(w,-h,z);
    #endif
    glEnd();

    glPopMatrix();
}

void Viewer::drawLandmark() {
    std::lock_guard<std::mutex> lockLandmark(landmarkMutex);

    if (!readyToDrawLandmark) return;

    glPointSize(landmarkSize);
    glBegin(GL_POINTS);
    glColor3f(0.2f, 0.2f, 0.6f);
    for (auto& pair : pMapPoints) {
        Eigen::Vector3d& point = pair.second->position;
        glVertex3f(point.x(), point.y(), point.z());
    }
    glEnd();
}

void Viewer::drawLoopConnection() {
    std::lock_guard<std::mutex> lockLoop(loopMutex);

    if (!readyToDrawLoop) return;

    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glColor3f(0.2f, 0.4f, 0.4f);
    for (unsigned i = 0; i < loopConnection.size(); i++) {
        int idx1 = loopConnection[i].first;
        int idx2 = loopConnection[i].second;
        glVertex3f(positions[idx1].x(), positions[idx1].y(), positions[idx1].z());
        glVertex3f(positions[idx1].x(), positions[idx1].y(), positions[idx1].z());
    }
    glEnd();
}

void Viewer::drawFullBAPosition() {
    std::lock_guard<std::mutex> lockPosition(fullBAPositionMutex);

    if (!readyToDrawFullBAPosition) return;

    glPointSize(pointSize);
    glColor3f(0.1f, 0.1f, 0.8f);
    glBegin(GL_POINTS);
    for (int i = 0; i < fullBAPositions.size(); i++)
        glVertex3f(fullBAPositions[i].x(), fullBAPositions[i].y(), fullBAPositions[i].z());
    glEnd();

    glLineWidth(lineWidth);
    glColor3f(0.1f, 0.1f, 0.8f);
    glBegin(GL_LINES);
    glVertex3f(fullBAPositions[0].x(), fullBAPositions[0].y(), fullBAPositions[0].z());
    for (int i = 1; i < fullBAPositions.size() - 1; i++) {
        glVertex3f(fullBAPositions[i].x(), fullBAPositions[i].y(), fullBAPositions[i].z());
        glVertex3f(fullBAPositions[i].x(), fullBAPositions[i].y(), fullBAPositions[i].z());
    }
    glVertex3f(fullBAPositions.back().x(), fullBAPositions.back().y(), fullBAPositions.back().z());
    glEnd();
}

} // namespace cfsd

#endif // USE_VIEWER