#pragma once

#include "Transform.h"
#include "Frustum.h"
#include "Rectangle.h"
#include "Properties.h"


namespace gameplay
{
    class Node;


    /**
     * Defines a camera which acts as a view of a scene to be rendered.
     */
    class Camera : public Transform::Listener
    {
        friend class Node;

    public:
        /**
         * Creates a perspective camera.
         *
         * @param fieldOfView The field of view in degrees for the perspective camera (normally in the range of 40-60 degrees).
         * @param aspectRatio The aspect ratio of the camera (normally the width of the viewport divided by the height of the viewport).
         * @param nearPlane The near plane distance.
         * @param farPlane The far plane distance.
         */
        Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);

        /**
         * Creates an orthographic camera.
         *
         * @param zoomX The zoom factor along the X-axis of the orthographic projection (the width of the ortho projection).
         * @param zoomY The zoom factor along the Y-axis of the orthographic projection (the height of the ortho projection).
         * @param aspectRatio The aspect ratio of the orthographic projection.
         * @param nearPlane The near plane distance.
         * @param farPlane The far plane distance.
         */
        Camera(float zoomX, float zoomY, float aspectRatio, float nearPlane, float farPlane);

        virtual ~Camera();


        /**
         * The type of camera.
         */
        enum Type
        {
            PERSPECTIVE = 1,
            ORTHOGRAPHIC = 2
        };


        /**
         * Listener interface for camera events.
         */
        class Listener
        {
        public:

            virtual ~Listener()
            {
            }


            /**
             * Handles when an camera settings change or the transform changed for the node its attached to.
             *
             * @param camera The camera that was changed.
             */
            virtual void cameraChanged(Camera* camera) = 0;
        };


        /**
         * Gets the type of camera.
         *
         * @return The camera type.
         */
        Camera::Type getCameraType() const;

        /**
         * Gets the field of view for a perspective camera.
         *
         * @return The field of view.
         */
        float getFieldOfView() const;

        /**
         * Sets the field of view.
         *
         * @param fieldOfView The field of view.
         */
        void setFieldOfView(float fieldOfView);

        /**
         * Gets the x-zoom (magnification) for an orthographic camera.
         * Default is 1.0f.
         *
         * @return The magnification (zoom) for x.
         */
        float getZoomX() const;

        /**
         * Sets the x-zoom (magnification) for a orthographic camera.
         * Default is 1.0f.
         *
         * @param zoomX The magnification (zoom) for x.
         */
        void setZoomX(float zoomX);

        /**
         * Gets the y-zoom (magnification) for a orthographic camera.
         * Default is 1.0f.
         *
         * @return The magnification (zoom) for y.
         */
        float getZoomY() const;

        /**
         * Sets the y-zoom (magnification) for a orthographic camera.
         *
         * @param zoomY The magnification (zoom) for y.
         */
        void setZoomY(float zoomY);

        /**
         * Gets the aspect ratio.
         *
         * @return The aspect ratio.
         */
        float getAspectRatio() const;

        /**
         * Sets the aspect ratio.
         *
         * @param aspectRatio The aspect ratio.
         */
        void setAspectRatio(float aspectRatio);

        /**
         * Gets the near z clipping plane distance.
         *
         * @return The near z clipping plane distance.
         */
        float getNearPlane() const;

        /**
         * Sets the near z clipping plane distance.
         *
         * @param nearPlane The near z clipping plane distance.
         */
        void setNearPlane(float nearPlane);

        /**
         * Gets the far z clipping plane distance.
         *
         * @return The far z clipping plane distance.
         */
        float getFarPlane() const;

        /**
         * Sets the far z clipping plane distance.
         *
         * @param farPlane The far z clipping plane distance.
         */
        void setFarPlane(float farPlane);

        /**
         * Gets the node that this camera is attached to.
         *
         * @return The node that this camera is attached to.
         */
        const std::shared_ptr<Node>& getNode() const;

        /**
         * Gets the camera's view matrix.
         *
         * @return The camera view matrix.
         */
        const glm::mat4& getViewMatrix() const;

        /**
         * Gets the camera's inverse view matrix.
         *
         * @return The camera inverse view matrix.
         */
        const glm::mat4& getInverseViewMatrix() const;

        /**
         * Gets the camera's projection matrix.
         *
         * @return The camera projection matrix.
         */
        const glm::mat4& getProjectionMatrix() const;

        /**
         * Sets a custom projection matrix to be used by the camera.
         *
         * Setting a custom projection matrix results in the internally
         * computed projection matrix being completely overridden until
         * the resetProjectionMatrix method is called. A custom projection
         * matrix is normally not necessary, but can be used for special
         * projection effects, such as setting an oblique view frustum
         * for near plane clipping.
         *
         * @param matrix Custom projection matrix.
         */
        void setProjectionMatrix(const glm::mat4& matrix);

        /**
         * Resets the camera to use the internally computed projection matrix
         * instead of any previously specified user-defined matrix.
         */
        void resetProjectionMatrix();

        /**
         * Gets the camera's view * projection matrix.
         *
         * @return The camera view * projection matrix.
         */
        const glm::mat4& getViewProjectionMatrix() const;

        /**
         * Gets the camera's inverse view * projection matrix.
         *
         * @return The camera inverse view * projection matrix.
         */
        const glm::mat4& getInverseViewProjectionMatrix() const;

        /**
         * Gets the view bounding frustum.
         *
         * @return The viewing bounding frustum.
         */
        const Frustum& getFrustum() const;

        /**
         * Projects the specified world position into the viewport coordinates.
         *
         * @param viewport The viewport rectangle to use.
         * @param position The world space position.
         * @param x The returned viewport x coordinate.
         * @param y The returned viewport y coordinate.
         * @param depth The returned pixel depth (can be nullptr).
         *
         * @script{ignore}
         */
        void project(const Rectangle& viewport, const glm::vec3& position, float* x, float* y, float* depth = nullptr) const;

        /**
         * Projects the specified world position into the viewport coordinates.
         *
         * @param viewport The viewport rectangle to use.
         * @param position The world space position.
         * @param out Populated with the resulting screen-space position.
         */
        void project(const Rectangle& viewport, const glm::vec3& position, glm::vec2* out) const;

        /**
         * Projects the specified world position into the viewport coordinates.
         *
         * @param viewport The viewport rectangle to use.
         * @param position The world space position.
         * @param out Populated with the resulting screen-space position, with the pixel depth in the Z coordinate.
         */
        void project(const Rectangle& viewport, const glm::vec3& position, glm::vec3* out) const;

        /**
         * Converts a viewport-space coordinate to a world-space position for the given depth value.
         *
         * The depth parameter is a value ranging between 0 and 1, where 0 returns a point on the
         * near clipping plane and 1 returns a point on the far clipping plane.
         *
         * @param viewport The viewport rectangle to use.
         * @param x The viewport-space x coordinate.
         * @param y The viewport-space y coordinate.
         * @param depth The depth range.
         * @param dst The world space position.
         */
        void unproject(const Rectangle& viewport, float x, float y, float depth, glm::vec3* dst) const;

        /**
         * Picks a ray that can be used for picking given the specified viewport-space coordinates.
         *
         * @param viewport The viewport rectangle to use.
         * @param x The viewport x-coordinate.
         * @param y The viewport y-coordinate.
         * @param dst The computed pick ray.
         */
        void pickRay(const Rectangle& viewport, float x, float y, Ray* dst) const;

        /**
        * Adds a camera listener.
        *
        * @param listener The listener to add.
        */
        void addListener(Camera::Listener* listener);

        /**
         * Removes a camera listener.
         *
         * @param listener The listener to remove.
         */
        void removeListener(Camera::Listener* listener);

    private:

        Camera& operator=(const Camera&) = delete;

        /**
         * Sets the node associated with this camera.
         */
        void setNode(const std::shared_ptr<Node>& node);

        /**
         * @see Transform::Listener::transformChanged
         */
        void transformChanged(Transform* transform, long cookie) override;

        void cameraChanged();

        Camera::Type _type;
        float _fieldOfView;
        float _zoom[2];
        float _aspectRatio;
        float _nearPlane;
        float _farPlane;
        mutable glm::mat4 _view;
        mutable glm::mat4 _projection;
        mutable glm::mat4 _viewProjection;
        mutable glm::mat4 _inverseView;
        mutable glm::mat4 _inverseViewProjection;
        mutable Frustum _bounds;
        mutable int _bits;
        std::shared_ptr<Node> _node;
        std::list<Camera::Listener*> _listeners;
    };
}
