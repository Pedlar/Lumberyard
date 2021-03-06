/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <EMotionFX/Source/Skeleton.h>
#include <Source/Editor/SimulatedObjectModel.h>
#include <EMotionFX/CommandSystem/Source/CommandManager.h>
#include <EMotionFX/CommandSystem/Source/SimulatedObjectCommands.h>
#include <Source/Editor/QtMetaTypes.h>

namespace EMotionFX
{
    bool SimulatedObjectModel::CommandAddSimulatedObjectPreCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->PreAddObject();

        return true;
    }

    bool SimulatedObjectModel::CommandAddSimulatedObjectPreCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(commandLine);
        CommandRemoveSimulatedObject* commandRemoveSimulatedObject = static_cast<CommandRemoveSimulatedObject*>(command);
        m_simulatedObjectModel->PreRemoveObject(commandRemoveSimulatedObject->GetObjectIndex());

        return true;
    }

    bool SimulatedObjectModel::CommandAddSimulatedObjectPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->PostAddObject();
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandAddSimulatedObjectPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);

        m_simulatedObjectModel->PostRemoveObject();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedObjectPreCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(commandLine);

        CommandRemoveSimulatedObject* commandRemoveSimulatedObject = static_cast<CommandRemoveSimulatedObject*>(command);
        m_simulatedObjectModel->PreRemoveObject(commandRemoveSimulatedObject->GetObjectIndex());

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedObjectPreCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->PreAddObject();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedObjectPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);

        m_simulatedObjectModel->PostRemoveObject();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedObjectPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->PostAddObject();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandAdjustSimulatedObjectPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(commandLine);

        CommandAdjustSimulatedObject* commandAdjustSimulatedObject = static_cast<CommandAdjustSimulatedObject*>(command);
        const QModelIndex objectIndex = m_simulatedObjectModel->index(static_cast<int>(commandAdjustSimulatedObject->GetObjectIndex()), 0);
        m_simulatedObjectModel->dataChanged(objectIndex, objectIndex.sibling(objectIndex.row(), m_simulatedObjectModel->columnCount() - 1));
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandAdjustSimulatedObjectPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        return this->Execute(command, commandLine);
    }

    bool SimulatedObjectModel::CommandAddSimulatedJointsPreCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->beginResetModel();

        return true;
    }

    bool SimulatedObjectModel::CommandAddSimulatedJointsPreCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->beginResetModel();

        return true;
    }

    bool SimulatedObjectModel::CommandAddSimulatedJointsPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->endResetModel();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandAddSimulatedJointsPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->endResetModel();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedJointsPreCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->beginResetModel();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedJointsPreCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->beginResetModel();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedJointsPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->endResetModel();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandRemoveSimulatedJointsPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->endResetModel();
        m_simulatedObjectModel->OnModelModified();

        return true;
    }

    bool SimulatedObjectModel::CommandAdjustSimulatedJointPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(commandLine);

        CommandAdjustSimulatedJoint* commandAdjustSimulatedJoint = static_cast<CommandAdjustSimulatedJoint*>(command);
        const QModelIndexList foundIndexes = m_simulatedObjectModel->match(
            /*start=*/m_simulatedObjectModel->index(0, 0),
            /*role=*/SimulatedObjectModel::ROLE_JOINT_PTR,
            /*value=*/QVariant::fromValue(commandAdjustSimulatedJoint->GetSimulatedJoint()),
            /*hits=*/1,
            /*flags=*/Qt::MatchExactly | Qt::MatchRecursive
        );
        if (foundIndexes.empty())
        {
            return false;
        }
        const QModelIndex jointIndex = foundIndexes[0];
        m_simulatedObjectModel->dataChanged(jointIndex, jointIndex.sibling(jointIndex.row(), m_simulatedObjectModel->columnCount() - 1));
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandAdjustSimulatedJointPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        return this->Execute(command, commandLine);
    }

    bool SimulatedObjectModel::CommandAddColliderPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandAddColliderPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandAdjustColliderPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandAdjustColliderPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandRemoveColliderPostCallback::Execute(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->OnModelModified();
        return true;
    }

    bool SimulatedObjectModel::CommandRemoveColliderPostCallback::Undo(MCore::Command* command, const MCore::CommandLine& commandLine)
    {
        AZ_UNUSED(command);
        AZ_UNUSED(commandLine);
        m_simulatedObjectModel->OnModelModified();
        return true;
    }
} // namespace EMotionFX
