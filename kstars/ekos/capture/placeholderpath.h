/*
    SPDX-FileCopyrightText: 2021 Kwon-Young Choi <kwon-young.choi@hotmail.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <lilxml.h>
#include "indi/indistd.h"
#include <QDebug>
#include <QFileInfo>

class QString;
class SchedulerJob;

namespace Ekos
{

class SequenceJob;

class PlaceholderPath
{
    public:
        PlaceholderPath(const QString &seqFilename);
        PlaceholderPath();
        ~PlaceholderPath();

        /**
         * @brief processJobInfo loads the placeHolderPath with properties from the SequenceJob
         * @param sequence job to be processed
         * @param targetname name of the celestial target
         */
        void processJobInfo(SequenceJob *job, const QString &targetName);

        /**
         * @brief addjob creates the directory suffix for the SequenceJob
         * @param sequence job to be processed
         * @param targetname name of the celestial target
         */
        void addJob(SequenceJob *job, const QString &targetName);

        /**
         * @brief constructPrefix creates the prefix for the SequenceJob
         * @param sequence job to be processed
         * @param imagePrefix sequence job prefix string
         * @return QString containing the processed prefix string
         */
        QString constructPrefix(const SequenceJob *job, const QString &imagePrefix);

        /**
         * @brief generateFilename performs the data for tag substituion in the filename
         * @param sequence job to be processed
         * @param targetName name of the celestial target
         * @param batch_mode if true dateTime tag is returned with placeholders
         * @param nextSequenceID file sequence number count
         * @param extension filename extension
         * @param filename passed in with tags
         * @param glob used in batch mode
         * @return QString containing formatted filename
         *
         * This overload of the function supports calls from the capture class
         */
        QString generateFilename(const SequenceJob &job, const QString &targetName, const bool batch_mode, const int nextSequenceID,
                                 const QString &extension, const QString &filename, const bool glob = false, const bool gettingSignature = false) const;

        /**
         * @brief generateFilename performs the data for tag substituion in the filename
         * @param sequence job to be processed
         * @param batch_mode if true dateTime tag is returned with placeholders
         * @param nextSequenceID file sequence number count
         * @param extension filename extension
         * @param filename passed in with tags
         * @param glob used in batch mode
         * @return QString containing formatted filename
         *
         * This overload of the function supports calls from the indicamera class
         */
        QString generateFilename(const bool batch_mode, const int nextSequenceID, const QString &extension, const QString &filename,
                                 const bool glob = false, const bool gettingSignature = false) const;

        /**
         * @brief setGenerateFilenameSettings loads the placeHolderPath with settings from the passed job
         * @param sequence job to be processed
         */
        void setGenerateFilenameSettings(const SequenceJob &job);

        /**
         * @brief remainingPlaceholders finds placeholder tags in filename
         * @param filename string to be processed
         * @return a QStringList of the remaining placeholders
         */
        static QStringList remainingPlaceholders(const QString &filename);

        /**
         * @brief remainingPlaceholders provides a list of already existing fileIDs from passed sequence job
         * @param sequence job to be processed
         * @param targetName name of the celestial target
         * @return a QStringList of the existing fileIDs
         */
        QList<int> getCompletedFileIds(const SequenceJob &job, const QString &targetName);

        /**
         * @brief getCompletedFiles provides the number of existing fileIDs
         * @param sequence job to be processed
         * @param targetName name of the celestial target
         * @return number of existing fileIDs
         */
        int getCompletedFiles(const SequenceJob &job, const QString &targetName);

        /**
         * @brief checkSeqBoundary provides the ID to use for the next file
         * @param sequence job to be processed
         * @param targetName name of the celestial target
         * @return number for the next fileIDs
         */
        int checkSeqBoundary(const SequenceJob &job, const QString &targetName);

    private:
        QString generateFilename(const QString &format, const QString &rawFilePrefix, const bool isDarkFlat, const QString &filter, const CCDFrameType &frameType,
                                 const double exposure, const QString &targetName, const bool batch_mode, const int nextSequenceID, const QString &extension,
                                 const QString &filename, const bool glob = false, const bool gettingSignature = false) const;

        QString getFrameType(CCDFrameType frameType) const
        {
            if (m_frameTypes.contains(frameType))
                return m_frameTypes[frameType];

            qWarning() << frameType << " not in " << m_frameTypes.keys();
            return "";
        }

        QMap<CCDFrameType, QString> m_frameTypes;
        QFileInfo m_seqFilename;
        QString m_format;
        bool m_tsEnabled { false };
        QString m_RawPrefix;
        bool m_filterPrefixEnabled { false };
        bool m_expPrefixEnabled { false };
        bool m_DarkFlat {false};
        QString m_filter;
        CCDFrameType m_frameType { FRAME_LIGHT };
        double m_exposure { -1 };
        QString m_targetName;
};

}

