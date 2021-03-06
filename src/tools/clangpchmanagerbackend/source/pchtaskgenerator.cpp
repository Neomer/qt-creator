/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "pchtaskgenerator.h"

#include "builddependenciesproviderinterface.h"
#include "pchtasksmergerinterface.h"

#include "usedmacrofilter.h"

#include <utils/algorithm.h>

namespace ClangBackEnd {

void PchTaskGenerator::addProjectParts(ProjectPartContainers &&projectParts,
                                       Utils::SmallStringVector &&toolChainArguments)
{
    PchTaskSets pchTaskSets;
    pchTaskSets.reserve(projectParts.size());

    for (auto &projectPart : projectParts) {
        BuildDependency buildDependency = m_buildDependenciesProvider.create(projectPart);
        UsedMacroFilter filter{buildDependency.includes, buildDependency.usedMacros};

        filter.filter(projectPart.compilerMacros);

        pchTaskSets.emplace_back(PchTask{projectPart.projectPartId.clone(),
                                         std::move(filter.topSystemIncludes),
                                         {},
                                         std::move(filter.systemCompilerMacros),
                                         std::move(filter.systemUsedMacros),
                                         projectPart.toolChainArguments,
                                         projectPart.systemIncludeSearchPaths,
                                         projectPart.projectIncludeSearchPaths,
                                         projectPart.language,
                                         projectPart.languageVersion,
                                         projectPart.languageExtension},
                                 PchTask{std::move(projectPart.projectPartId),
                                         std::move(filter.topProjectIncludes),
                                         std::move(filter.allIncludes),
                                         std::move(filter.projectCompilerMacros),
                                         std::move(filter.projectUsedMacros),
                                         projectPart.toolChainArguments,
                                         projectPart.systemIncludeSearchPaths,
                                         projectPart.projectIncludeSearchPaths,
                                         projectPart.language,
                                         projectPart.languageVersion,
                                         projectPart.languageExtension});
    }

    m_pchTasksMergerInterface.mergeTasks(std::move(pchTaskSets), std::move(toolChainArguments));
}

void PchTaskGenerator::removeProjectParts(const Utils::SmallStringVector &projectsPartIds)
{
    m_pchTasksMergerInterface.removePchTasks(projectsPartIds);
}

} // namespace ClangBackEnd
