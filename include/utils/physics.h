#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

enum Shapes
{
    BOX,
    SPHERE
};

class Physics
{
private:
public:
    btDiscreteDynamicsWorld *dynamicWorld;
    btAlignedObjectArray<btCollisionShape *> collisionShapes;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btBroadphaseInterface *overlappingPairCache;
    btSequentialImpulseConstraintSolver *solver;

    Physics()
    {
        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        overlappingPairCache = new btDbvtBroadphase();

        solver = new btSequentialImpulseConstraintSolver();
        dynamicWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

        dynamicWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
    }

    btRigidBody *CreateRigidBody(Shapes type, glm::vec3 pos, glm::vec3 size, glm::vec3 rot, float m, float friction, float restitution)
    {
        btCollisionShape *cShape = nullptr;
        btVector3 position = btVector3(pos.x, pos.y, pos.z);
        btQuaternion rotation;
        rotation.setEuler(rot.x, rot.y, rot.z);

        if (type == BOX)
        {
            btVector3 dim = btVector3(size.x, size.y, size.z);
            cShape = new btBoxShape(dim);
        }
        else
        {
            cShape = new btSphereShape(size.x);
        }
        collisionShapes.push_back(cShape);

        btTransform objTransform;
        objTransform.setIdentity();
        objTransform.setRotation(rotation);
        objTransform.setOrigin(position);

        btScalar mass = m;
        bool isDynamic = (mass != 0.0f);

        btVector3 localInertia(0.0f, 0.0f, 0.0f);
        if (isDynamic)
            cShape->calculateLocalInertia(mass, localInertia);

        btDefaultMotionState *motionState = new btDefaultMotionState(objTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, cShape, localInertia);
        rbInfo.m_friction = friction;
        rbInfo.m_restitution = restitution;

        if (type == SPHERE)
        {
            rbInfo.m_angularDamping = 0.3f;
            rbInfo.m_rollingFriction = 0.3f;
        }

        btRigidBody *body = new btRigidBody(rbInfo);
        dynamicWorld->addRigidBody(body);

        return body;
    }

    void Clear()
    {
        for (int i = dynamicWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject *obj = dynamicWorld->getCollisionObjectArray()[i];
            btRigidBody *body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            dynamicWorld->removeCollisionObject(obj);
        }

        for (int i = 0; i < collisionShapes.size(); i++)
        {
            btCollisionShape *shape = collisionShapes[i];
            collisionShapes[i] = nullptr;
            delete shape;
        }

        delete dynamicWorld;
        delete solver;
        delete overlappingPairCache;
        delete dispatcher;
        delete collisionConfiguration;

        collisionShapes.clear();
    }
};
