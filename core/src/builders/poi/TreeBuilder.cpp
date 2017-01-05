#include <mapcss/StyleConsts.hpp>
#include "builders/poi/TreeBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/MeshUtils.hpp"
#include "lsys/LSystem.hpp"

using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const std::string NodeMeshNamePrefix = "tree:";
    const std::string WayMeshNamePrefix = "trees:";

    const std::string TreeStepKey = "tree-step";
}

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(utymap::utils::getMeshName(NodeMeshNamePrefix, node));
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);

    const auto& lsystem = context_.styleProvider.getLsystem(style.getString(StyleConsts::LSystemKey()));
    double elevation = context_.eleProvider.getElevation(context_.quadKey, node.coordinate);
    
    TreeGenerator(context_, style, mesh)
        .setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .run(lsystem);

    context_.meshCallback(mesh);
}

void TreeBuilder::visitWay(const utymap::entities::Way& way)
{
    Mesh treeMesh("");
    Mesh newMesh(utymap::utils::getMeshName(WayMeshNamePrefix, way));

    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);
    const auto& lsystem = context_.styleProvider.getLsystem(style.getString(StyleConsts::LSystemKey()));

    TreeGenerator(context_, style, treeMesh)
        .setPosition(Vector3(0, 0, 0))// NOTE we will override coordinates later
        .run(lsystem);

    double treeStepInMeters = style.getValue(TreeStepKey);

    for (std::size_t i = 0; i < way.coordinates.size() - 1; ++i) {
        const auto& p0 = way.coordinates[i];
        const auto& p1 = way.coordinates[i + 1];
        utymap::utils::copyMeshAlong(context_.quadKey, p0, p1, treeMesh, newMesh, treeStepInMeters, context_.eleProvider);
    }

    context_.meshCallback(newMesh);
}

void TreeBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    for (const auto& element : relation.elements) {
        element->accept(*this);
    }
}
