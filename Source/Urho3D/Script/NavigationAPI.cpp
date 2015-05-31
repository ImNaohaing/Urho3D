//
// Copyright (c) 2008-2015 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifdef URHO3D_NAVIGATION

#include "../Precompiled.h"

#include "../Navigation/Navigable.h"
#include "../Navigation/CrowdAgent.h"
#include "../Navigation/NavigationMesh.h"
#include "../Navigation/DynamicNavigationMesh.h"
#include "../Navigation/OffMeshConnection.h"
#include "../Navigation/NavArea.h"
#include "../Navigation/Obstacle.h"
#include "../Script/APITemplates.h"

namespace Urho3D
{

void RegisterNavigable(asIScriptEngine* engine)
{
    RegisterComponent<Navigable>(engine, "Navigable");
    engine->RegisterObjectMethod("Navigable", "void set_recursive(bool)", asMETHOD(Navigable, SetRecursive), asCALL_THISCALL);
    engine->RegisterObjectMethod("Navigable", "bool get_recursive() const", asMETHOD(Navigable, IsRecursive), asCALL_THISCALL);
}

static CScriptArray* NavigationMeshFindPath(const Vector3& start, const Vector3& end, const Vector3& extents, NavigationMesh* ptr)
{
    PODVector<Vector3> dest;
    ptr->FindPath(dest, start, end, extents);
    return VectorToArray<Vector3>(dest, "Array<Vector3>");
}

static CScriptArray* DynamicNavigationMeshFindPath(const Vector3& start, const Vector3& end, const Vector3& extents, DynamicNavigationMesh* ptr)
{
    PODVector<Vector3> dest;
    ptr->FindPath(dest, start, end, extents);
    return VectorToArray<Vector3>(dest, "Array<Vector3>");
}

static CScriptArray* DetourCrowdManagerGetActiveAgents(DetourCrowdManager* crowd)
{
    const PODVector<CrowdAgent*>& agents = crowd->GetActiveAgents();
    return VectorToHandleArray<CrowdAgent>(agents, "Array<CrowdAgent@>");
}

template<class T> static void RegisterNavMeshBase(asIScriptEngine* engine, const char* name)
{
    engine->RegisterObjectMethod(name, "bool Build()", asMETHODPR(T, Build, (void), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool Build(const BoundingBox&in)", asMETHODPR(T, Build, (const BoundingBox&), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void SetAreaCost(uint, float)", asMETHOD(T, SetAreaCost), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float GetAreaCost(uint) const", asMETHOD(T, GetAreaCost), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "Vector3 FindNearestPoint(const Vector3&in, const Vector3&in extents = Vector3(1.0, 1.0, 1.0))", asMETHOD(T, FindNearestPoint), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "Vector3 MoveAlongSurface(const Vector3&in, const Vector3&in, const Vector3&in extents = Vector3(1.0, 1.0, 1.0), uint = 3)", asMETHOD(T, MoveAlongSurface), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "Vector3 GetRandomPoint()", asMETHOD(T, GetRandomPoint), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "Vector3 GetRandomPointInCircle(const Vector3&in, float, const Vector3&in extents = Vector3(1.0, 1.0, 1.0))", asMETHOD(T, GetRandomPointInCircle), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float GetDistanceToWall(const Vector3&in, float, const Vector3&in extents = Vector3(1.0, 1.0, 1.0))", asMETHOD(T, GetDistanceToWall), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "Vector3 Raycast(const Vector3&in, const Vector3&in, const Vector3&in extents = Vector3(1.0, 1.0, 1.0))", asMETHOD(T, Raycast), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void DrawDebugGeometry(bool)", asMETHODPR(NavigationMesh, DrawDebugGeometry, (bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_tileSize(int)", asMETHOD(T, SetTileSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "int get_tileSize() const", asMETHOD(T, GetTileSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_cellSize(float)", asMETHOD(T, SetCellSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_cellSize() const", asMETHOD(T, GetCellSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_cellHeight(float)", asMETHOD(T, SetCellHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_cellHeight() const", asMETHOD(T, GetCellHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_agentHeight(float)", asMETHOD(T, SetAgentHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_agentHeight() const", asMETHOD(T, GetAgentHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_agentRadius(float)", asMETHOD(T, SetAgentRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_agentRadius() const", asMETHOD(T, GetAgentRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_agentMaxClimb(float)", asMETHOD(T, SetAgentMaxClimb), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_agentMaxClimb() const", asMETHOD(T, GetAgentMaxClimb), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_agentMaxSlope(float)", asMETHOD(T, SetAgentMaxSlope), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_agentMaxSlope() const", asMETHOD(T, GetAgentMaxSlope), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_regionMinSize(float)", asMETHOD(T, SetRegionMinSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_regionMinSize() const", asMETHOD(T, GetRegionMinSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_regionMergeSize(float)", asMETHOD(T, SetRegionMergeSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_regionMergeSize() const", asMETHOD(T, GetRegionMergeSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_edgeMaxLength(float)", asMETHOD(T, SetEdgeMaxLength), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_edgeMaxLength() const", asMETHOD(T, GetEdgeMaxLength), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_edgeMaxError(float)", asMETHOD(T, SetEdgeMaxError), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_edgeMaxError() const", asMETHOD(T, GetEdgeMaxError), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_detailSampleDistance(float)", asMETHOD(T, SetDetailSampleDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_detailSampleDistance() const", asMETHOD(T, GetDetailSampleDistance), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_detailSampleMaxError(float)", asMETHOD(T, SetDetailSampleMaxError), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "float get_detailSampleMaxError() const", asMETHOD(T, GetDetailSampleMaxError), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_padding(const Vector3&in)", asMETHOD(T, SetPadding), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "const Vector3& get_padding() const", asMETHOD(T, GetPadding), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool get_initialized() const", asMETHOD(T, IsInitialized), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "const BoundingBox& get_boundingBox() const", asMETHOD(T, GetBoundingBox), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "BoundingBox get_worldBoundingBox() const", asMETHOD(T, GetWorldBoundingBox), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "IntVector2 get_numTiles() const", asMETHOD(T, GetNumTiles), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_partitionType()", asMETHOD(T, SetPartitionType), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "NavmeshPartitionType get_partitionType()", asMETHOD(T, GetPartitionType), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_drawOffMeshConnections(bool)", asMETHOD(T, SetDrawOffMeshConnections), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool get_drawOffMeshConnections() const", asMETHOD(T, GetDrawOffMeshConnections), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void set_drawNavAreas(bool)", asMETHOD(T, SetDrawNavAreas), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool get_drawNavAreas() const", asMETHOD(T, GetDrawNavAreas), asCALL_THISCALL);
}

void RegisterNavigationMesh(asIScriptEngine* engine)
{
    engine->RegisterEnum("NavmeshPartitionType");
    engine->RegisterEnumValue("NavmeshPartitionType", "NAVMESH_PARTITION_WATERSHED", NAVMESH_PARTITION_WATERSHED);
    engine->RegisterEnumValue("NavmeshPartitionType", "NAVMESH_PARTITION_MONOTONE", NAVMESH_PARTITION_MONOTONE);

    RegisterComponent<NavigationMesh>(engine, "NavigationMesh");
    RegisterNavMeshBase<NavigationMesh>(engine, "NavigationMesh");
    engine->RegisterObjectMethod("NavigationMesh", "Array<Vector3>@ FindPath(const Vector3&in, const Vector3&in, const Vector3&in extents = Vector3(1.0, 1.0, 1.0))", asFUNCTION(NavigationMeshFindPath), asCALL_CDECL_OBJLAST);
}

void RegisterDynamicNavigationMesh(asIScriptEngine* engine)
{
    RegisterComponent<DynamicNavigationMesh>(engine, "DynamicNavigationMesh");
    RegisterSubclass<NavigationMesh, DynamicNavigationMesh>(engine, "NavigationMesh", "DynamicNavigationMesh");
    RegisterNavMeshBase<DynamicNavigationMesh>(engine, "DynamicNavigationMesh");
    engine->RegisterObjectMethod("DynamicNavigationMesh", "Array<Vector3>@ FindPath(const Vector3&in, const Vector3&in, const Vector3&in extents = Vector3(1.0, 1.0, 1.0))", asFUNCTION(DynamicNavigationMeshFindPath), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("DynamicNavigationMesh", "void set_drawObstacles(bool)", asMETHOD(DynamicNavigationMesh, SetDrawObstacles), asCALL_THISCALL);
    engine->RegisterObjectMethod("DynamicNavigationMesh", "bool get_drawObstacles() const", asMETHOD(DynamicNavigationMesh, GetDrawObstacles), asCALL_THISCALL);
    engine->RegisterObjectMethod("DynamicNavigationMesh", "void set_maxObstacles(uint)", asMETHOD(DynamicNavigationMesh, SetMaxObstacles), asCALL_THISCALL);
    engine->RegisterObjectMethod("DynamicNavigationMesh", "uint get_maxObstacles() const", asMETHOD(DynamicNavigationMesh, GetMaxObstacles), asCALL_THISCALL);
}

void RegisterOffMeshConnection(asIScriptEngine* engine)
{
    RegisterComponent<OffMeshConnection>(engine, "OffMeshConnection");
    engine->RegisterObjectMethod("OffMeshConnection", "void set_endPoint(Node@+)", asMETHOD(OffMeshConnection, SetEndPoint), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "Node@+ get_endPoint() const", asMETHOD(OffMeshConnection, GetEndPoint), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "void set_radius(float)", asMETHOD(OffMeshConnection, SetRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "float get_radius() const", asMETHOD(OffMeshConnection, GetRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "void set_bidirectional(bool)", asMETHOD(OffMeshConnection, SetBidirectional), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "bool get_bidirectional() const", asMETHOD(OffMeshConnection, IsBidirectional), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "void set_mask(uint)", asMETHOD(OffMeshConnection, SetMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "uint get_mask() const", asMETHOD(OffMeshConnection, GetMask), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "void set_areaID(uint)", asMETHOD(OffMeshConnection, SetAreaID), asCALL_THISCALL);
    engine->RegisterObjectMethod("OffMeshConnection", "uint get_areaID() const", asMETHOD(OffMeshConnection, GetAreaID), asCALL_THISCALL);
}

void RegisterObstacle(asIScriptEngine* engine)
{
    RegisterComponent<Obstacle>(engine, "Obstacle");
    engine->RegisterObjectMethod("Obstacle", "float get_radius() const", asMETHOD(Obstacle, GetRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod("Obstacle", "void set_radius(float)", asMETHOD(Obstacle, SetRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod("Obstacle", "float get_height() const", asMETHOD(Obstacle, GetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("Obstacle", "void set_height(float)", asMETHOD(Obstacle, SetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("Obstacle", "uint get_obstacleId() const", asMETHOD(Obstacle, GetObstacleID), asCALL_THISCALL);
    engine->RegisterObjectMethod("Obstacle", "void DrawDebugGeometry(bool)", asMETHODPR(Obstacle, DrawDebugGeometry, (bool), void), asCALL_THISCALL);
}

void RegisterNavArea(asIScriptEngine* engine)
{
    RegisterComponent<NavArea>(engine, "NavArea");
    engine->RegisterObjectMethod("NavArea", "BoundingBox get_boundingBox() const", asMETHOD(NavArea, GetBoundingBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("NavArea", "void set_boundingBox(const BoundingBox&in)", asMETHOD(NavArea, SetBoundingBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("NavArea", "uint get_areaID() const", asMETHOD(NavArea, GetAreaID), asCALL_THISCALL);
    engine->RegisterObjectMethod("NavArea", "void set_areaID(uint)", asMETHOD(NavArea, SetAreaID), asCALL_THISCALL);
    engine->RegisterObjectMethod("NavArea", "BoundingBox get_worldBoundingBox() const", asMETHOD(NavArea, GetWorldBoundingBox), asCALL_THISCALL);
}

void RegisterDetourCrowdManager(asIScriptEngine* engine)
{
    RegisterComponent<DetourCrowdManager>(engine, "DetourCrowdManager");
    engine->RegisterObjectMethod("DetourCrowdManager", "void CreateCrowd()", asMETHOD(DetourCrowdManager, CreateCrowd), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "void DrawDebugGeometry(bool)", asMETHODPR(DetourCrowdManager, DrawDebugGeometry, (bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "void set_navMesh(NavigationMesh@+)", asMETHOD(DetourCrowdManager, SetNavigationMesh), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "NavigationMesh@+ get_navMesh() const", asMETHOD(DetourCrowdManager, GetNavigationMesh), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "int get_maxAgents() const", asMETHOD(DetourCrowdManager, GetMaxAgents), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "void set_maxAgents(int)", asMETHOD(DetourCrowdManager, SetMaxAgents), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "Array<CrowdAgent@>@ GetActiveAgents()", asFUNCTION(DetourCrowdManagerGetActiveAgents), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod("DetourCrowdManager", "void SetAreaCost(uint, uint, float)", asMETHOD(DetourCrowdManager, SetAreaCost), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "float GetAreaCost(uint, uint)", asMETHOD(DetourCrowdManager, GetAreaCost), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "void SetCrowdTarget(const Vector3&in, int startId = 0, int endId = M_MAX_INT)", asMETHOD(DetourCrowdManager, SetCrowdTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "void SetCrowdVelocity(const Vector3&in, int startId = 0, int endId = M_MAX_INT)", asMETHOD(DetourCrowdManager, SetCrowdVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("DetourCrowdManager", "void ResetCrowdTarget(int startId = 0, int endId = M_MAX_INT)", asMETHOD(DetourCrowdManager, ResetCrowdTarget), asCALL_THISCALL);
}

void RegisterCrowdAgent(asIScriptEngine* engine)
{
    engine->RegisterEnum("CrowdAgentRequestedTarget");
    engine->RegisterEnumValue("CrowdAgentRequestedTarget", "CA_REQUESTEDTARGET_NONE", CA_REQUESTEDTARGET_NONE);
    engine->RegisterEnumValue("CrowdAgentRequestedTarget", "CA_REQUESTEDTARGET_POSITION", CA_REQUESTEDTARGET_POSITION);
    engine->RegisterEnumValue("CrowdAgentRequestedTarget", "CA_REQUESTEDTARGET_VELOCITY", CA_REQUESTEDTARGET_VELOCITY);

    engine->RegisterEnum("CrowdAgentTargetState");
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_NONE", CA_TARGET_NONE);
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_FAILED", CA_TARGET_FAILED);
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_VALID", CA_TARGET_VALID);
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_REQUESTING", CA_TARGET_REQUESTING);
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_WAITINGFORQUEUE", CA_TARGET_WAITINGFORQUEUE);
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_WAITINGFORPATH", CA_TARGET_WAITINGFORPATH);
    engine->RegisterEnumValue("CrowdAgentTargetState", "CA_TARGET_VELOCITY", CA_TARGET_VELOCITY);

    engine->RegisterEnum("CrowdAgentState");
    engine->RegisterEnumValue("CrowdAgentState", "CA_STATE_INVALID", CA_STATE_INVALID);
    engine->RegisterEnumValue("CrowdAgentState", "CA_STATE_WALKING", CA_STATE_WALKING);
    engine->RegisterEnumValue("CrowdAgentState", "CA_STATE_OFFMESH", CA_STATE_OFFMESH);

    engine->RegisterEnum("NavigationQuality");
    engine->RegisterEnumValue("NavigationQuality", "NAVIGATIONQUALITY_LOW", NAVIGATIONQUALITY_LOW);
    engine->RegisterEnumValue("NavigationQuality", "NAVIGATIONQUALITY_MEDIUM", NAVIGATIONQUALITY_MEDIUM);
    engine->RegisterEnumValue("NavigationQuality", "NAVIGATIONQUALITY_HIGH", NAVIGATIONQUALITY_HIGH);

    engine->RegisterEnum("NavigationPushiness");
    engine->RegisterEnumValue("NavigationPushiness", "NAVIGATIONPUSHINESS_LOW", NAVIGATIONPUSHINESS_LOW);
    engine->RegisterEnumValue("NavigationPushiness", "NAVIGATIONPUSHINESS_MEDIUM", NAVIGATIONPUSHINESS_MEDIUM);
    engine->RegisterEnumValue("NavigationPushiness", "NAVIGATIONPUSHINESS_HIGH", NAVIGATIONPUSHINESS_HIGH);

    RegisterComponent<CrowdAgent>(engine, "CrowdAgent");
    engine->RegisterObjectMethod("CrowdAgent", "void DrawDebugGeometry(bool)", asMETHODPR(CrowdAgent, DrawDebugGeometry, (bool), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void ResetTarget()", asMETHOD(CrowdAgent, ResetTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_updateNodePosition(bool)", asMETHOD(CrowdAgent, SetUpdateNodePosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "bool get_updateNodePosition() const", asMETHOD(CrowdAgent, GetUpdateNodePosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_targetPosition(const Vector3&in)", asMETHOD(CrowdAgent, SetTargetPosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "const Vector3& get_targetPosition()", asMETHOD(CrowdAgent, GetTargetPosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_targetVelocity(const Vector3&in)", asMETHOD(CrowdAgent, SetTargetVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "const Vector3& get_targetVelocity()", asMETHOD(CrowdAgent, GetTargetVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_maxAccel(float)", asMETHOD(CrowdAgent, SetMaxAccel), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "float get_maxAccel()", asMETHOD(CrowdAgent, GetMaxAccel), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_maxSpeed(float)", asMETHOD(CrowdAgent, SetMaxSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "float get_maxSpeed()", asMETHOD(CrowdAgent, GetMaxSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_radius(float)", asMETHOD(CrowdAgent, SetRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "float get_radius()", asMETHOD(CrowdAgent, GetRadius), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_height(float)", asMETHOD(CrowdAgent, SetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "float get_height()", asMETHOD(CrowdAgent, GetHeight), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "uint get_filterType()", asMETHOD(CrowdAgent, GetFilterType), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_filterType(uint)", asMETHOD(CrowdAgent, SetFilterType), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "uint get_obstacleAvoidanceType()", asMETHOD(CrowdAgent, GetObstacleAvoidanceType), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_obstacleAvoidanceType(uint)", asMETHOD(CrowdAgent, SetObstacleAvoidanceType), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_navigationQuality(NavigationQuality)", asMETHOD(CrowdAgent, SetNavigationQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "NavigationQuality get_navigationQuality()", asMETHOD(CrowdAgent, GetNavigationQuality), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "void set_navigationPushiness(NavigationPushiness)", asMETHOD(CrowdAgent, SetNavigationPushiness), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "NavigationPushiness get_navigationPushiness()", asMETHOD(CrowdAgent, GetNavigationPushiness), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "CrowdAgentRequestedTarget get_requestedTargetType() const", asMETHOD(CrowdAgent, GetRequestedTargetType), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "Vector3 get_position() const", asMETHOD(CrowdAgent, GetPosition), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "Vector3 get_desiredVelocity() const", asMETHOD(CrowdAgent, GetDesiredVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "Vector3 get_actualVelocity() const", asMETHOD(CrowdAgent, GetActualVelocity), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "CrowdAgentState get_agentState() const", asMETHOD(CrowdAgent, GetAgentState), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "CrowdAgentTargetState get_targetState() const", asMETHOD(CrowdAgent, GetTargetState), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "bool get_requestedTarget() const", asMETHOD(CrowdAgent, HasRequestedTarget), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "bool get_arrived() const", asMETHOD(CrowdAgent, HasArrived), asCALL_THISCALL);
    engine->RegisterObjectMethod("CrowdAgent", "bool get_inCrowd() const", asMETHOD(CrowdAgent, IsInCrowd), asCALL_THISCALL);
}

void RegisterNavigationAPI(asIScriptEngine* engine)
{
    RegisterNavigable(engine);
    RegisterNavigationMesh(engine);
    RegisterDynamicNavigationMesh(engine);
    RegisterOffMeshConnection(engine);
    RegisterCrowdAgent(engine);
    RegisterDetourCrowdManager(engine);
    RegisterObstacle(engine);
    RegisterNavArea(engine);
}

}

#endif
