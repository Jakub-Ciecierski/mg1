//
// Created by jakub on 3/5/16.
//

#ifndef MG1_SCENE_H
#define MG1_SCENE_H


#include <infinity_cad/rendering/render_objects/cross.h>
#include <infinity_cad/rendering/cameras/camera.h>
#include "infinity_cad/rendering/scene/scene_id_factory.h"

/*
 * Scene is responsible for deallocating all objects in the scene
 */
class Scene{
private:
    //-----------------------------------------------------------//
    //  PRIVATE FIELDS
    //-----------------------------------------------------------//

    Cross* cross;

    std::vector<RenderObject *> sceneObjects;

    std::vector<Camera*> cameras;
    Camera* activeCamera;

    Color sceneColor;

    bool rendering3DEnabled;

    SceneIDFactory sceneIDFactory;

    // Model of the scene, View and Projection from camera
    glm::mat4 MVP;

    void initSceneElements();

    void updateMVP();

    void renderAllObjects();

    void renderAllObjects3D();
public:
    //-----------------------------------------------------------//
    //  CONSTRUCTORS
    //-----------------------------------------------------------//

    Scene();

    ~Scene();

    //-----------------------------------------------------------//
    //  PUBLIC METHODS
    //-----------------------------------------------------------//

    //--------------------//
    //  SETTERS
    //--------------------//

    /*
     * After the object is added to the scene, the Scene takes ownership
     * and deallocates its memory when needed.
     *
     * Returns an unique ID of the object in the scene.
     */
    void addRenderObject(RenderObject * object);

    /*
     * The camera is taken to the ownership of the scene
     */
    void addCamera(Camera*);

    bool setActiveCamera(Camera *);
    bool setActiveCamera(const SceneID &);

    bool removeObject(RigidObject * object);
    bool removeObject(const SceneID& sceneID);

    void setColor(Color color);

    void set3DRendering(bool v);

    //--------------------//
    //  GETTERS
    //--------------------//

    const std::vector<RenderObject *>& getRenderObjects();
    const std::vector<Camera *>& getCameras();

    Camera* getActiveCamera();

    const Color& getColor();

    const glm::mat4& getMVP();

    RenderObject * getRenderBody(const SceneID& id);

    SceneID getNextAvailableID();

    RenderObject * getActiveRenderBody();

    Cross* getCross();

    void renderScene();

    void update();
};


#endif //MG1_SCENE_H