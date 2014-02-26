/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "componentatceditorwidget.h"
#include "componentatcmodel.h"
#include <ddiplugin/ddicore.h>
#include <ddiplugin/atc/searchatcindatabasedialog.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>

#include <utils/log.h>
#include <utils/global.h>
#include <translationutils/constants.h>
#include <translationutils/trans_drugs.h>
#include <translationutils/trans_filepathxml.h>
#include <translationutils/trans_msgerror.h>

#include <QFile>
#include <QList>
#include <QDate>
#include <QDesktopServices>
#include <QUrl>
#include <QMenu>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QFileDialog>

#include <QDebug>

#include "ui_componentatceditorwidget.h"

static inline Core::ISettings *settings() {return Core::ICore::instance()->settings();}
static inline DDI::DDICore *ddiCore() {return DDI::DDICore::instance();}

using namespace DDI;
using namespace Internal;
using namespace Trans::ConstantTranslations;

namespace DDI {
namespace Internal {
class ComponentAtcEditorWidgetPrivate
{
public:
    ComponentAtcEditorWidgetPrivate(ComponentAtcEditorWidget *parent) :
        ui(new Ui::ComponentAtcEditorWidget),
        model(0),
        proxyModel(0),
        q(parent)
    {
        Q_UNUSED(q);
    }

    ~ComponentAtcEditorWidgetPrivate()
    {
        delete ui;
    }

public:
    Ui::ComponentAtcEditorWidget *ui;
    ComponentAtcModel *model;
    QSortFilterProxyModel *proxyModel;

private:
    ComponentAtcEditorWidget *q;
};
} // namespace Internal
} // namespace DDI

ComponentAtcEditorWidget::ComponentAtcEditorWidget(QWidget *parent) :
    QWidget(parent),
    d(new ComponentAtcEditorWidgetPrivate(this))
{
    setObjectName("ComponentAtcEditorWidget");
    d->ui->setupUi(this);
    d->model = ddiCore()->componentAtcModel();
    d->model->fetchAll();
    d->ui->availableDrugsDb->addItems(d->model->availableDrugsDatabases());
    if (d->model->availableDrugsDatabases().count())
        d->model->selectDatabase(d->model->availableDrugsDatabases().at(0));

    d->proxyModel = new QSortFilterProxyModel(this);
    d->proxyModel->setSourceModel(d->model);
    d->proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    d->proxyModel->setFilterKeyColumn(ComponentAtcModel::Name);

    d->ui->tableView->setModel(d->proxyModel);
    d->ui->tableView->setSortingEnabled(true);
    d->ui->tableView->verticalHeader()->hide();
    d->ui->tableView->horizontalHeader()->setStretchLastSection(false);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::FancyButton, QHeaderView::Fixed);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::DrugDatabaseComponentUid1, QHeaderView::ResizeToContents);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::DrugDatabaseComponentUid2, QHeaderView::ResizeToContents);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::IsValid, QHeaderView::ResizeToContents);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::IsReviewed, QHeaderView::ResizeToContents);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::Name, QHeaderView::ResizeToContents);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::AtcCodeList, QHeaderView::Interactive);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::SuggestedAtcCodeList, QHeaderView::Interactive);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::DateCreation, QHeaderView::Interactive);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::DateUpdate, QHeaderView::Interactive);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::Reviewer, QHeaderView::Interactive);
    d->ui->tableView->horizontalHeader()->setResizeMode(ComponentAtcModel::Comments, QHeaderView::Interactive);
    d->ui->tableView->setColumnWidth(ComponentAtcModel::FancyButton, 24);
//    d->ui->tableView->horizontalHeader()->hideSection(ComponentAtcModel::Id);
    d->ui->tableView->horizontalHeader()->hideSection(ComponentAtcModel::Uid);
    d->ui->tableView->horizontalHeader()->hideSection(ComponentAtcModel::DrugDatabaseComponentUid1);
    d->ui->tableView->horizontalHeader()->hideSection(ComponentAtcModel::DrugDatabaseComponentUid2);

    connect(d->ui->availableDrugsDb, SIGNAL(activated(int)), this, SLOT(onChangeComponentDrugDatabaseUidRequested(int)));
    connect(d->ui->saveButton, SIGNAL(clicked()), this, SLOT(saveModel()));
    connect(d->ui->reveiwers, SIGNAL(activated(QString)), d->model, SLOT(setActualReviewer(QString)));
    connect(d->ui->tableView, SIGNAL(activated(QModelIndex)), this, SLOT(onComponentViewItemActivated(QModelIndex)));
    connect(d->ui->tableView, SIGNAL(pressed(QModelIndex)), this, SLOT(onComponentViewItemPressed(QModelIndex)));
    connect(d->ui->tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onComponentViewItemChanged(QModelIndex,QModelIndex)));
    connect(d->ui->createUnreviewed, SIGNAL(clicked()), this, SLOT(onCreateUnreviewedFileRequested()));
    connect(d->ui->searchMol, SIGNAL(textChanged(QString)), d->proxyModel, SLOT(setFilterWildcard(QString)));
    connect(d->model, SIGNAL(modelReset()), this, SLOT(onModelReset()));

    onModelReset();

//        processCSVFile();
}

ComponentAtcEditorWidget::~ComponentAtcEditorWidget()
{
    if (d)
        delete d;
    d = 0;
}

/**
 * Filter the Component model with the selected drug's database uid.
 * The drugs database uid is in the ComponentAtcModel::availableDrugsDatabases()
 */
void ComponentAtcEditorWidget::onChangeComponentDrugDatabaseUidRequested(const int index)
{
    const QStringList &dbUids = d->model->availableDrugsDatabases();
    if (!IN_RANGE_STRICT_MAX(index, 0, dbUids.count()))
        return;
    d->model->selectDatabase(dbUids.at(index));
    onModelReset();
}

void ComponentAtcEditorWidget::onComponentViewItemChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    d->ui->textBrowser->setHtml(current.data(Qt::ToolTipRole).toString());
}

/** Reacts on index is clicked on the component tableview */
void ComponentAtcEditorWidget::onComponentViewItemPressed(const QModelIndex &index)
{
    if (index.column() == ComponentAtcModel::FancyButton) {
        QMenu *menu = new QMenu(this);
        QAction *google = new QAction(tr("Search Google (copy molecule to clipboard)"), menu);
        QAction *who = new QAction(tr("Search WHO (copy molecule to clipboard)"), menu);
        QAction *resip = new QAction(tr("Search RESIP (copy molecule to clipboard)"), menu);
        QAction *cnamts = new QAction(tr("Search CNAMTS database"), menu);
        QAction *copyClip = new QAction(tr("Copy molecule name to clipboard"), menu);
        QAction *atcSearchDialog = new QAction(tr("Open the ATC search dialog"), menu);
        menu->addAction(atcSearchDialog);
        menu->addAction(google);
        menu->addAction(cnamts);
        menu->addAction(who);
        menu->addAction(resip);
        menu->addAction(copyClip);
        QAction *selected = menu->exec(QCursor::pos());
        if (selected == atcSearchDialog) {
            SearchAtcInDatabaseDialog dlg(this, d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString());
            if (dlg.exec() == QDialog::Accepted) {
                d->proxyModel->setData(d->proxyModel->index(index.row(), ComponentAtcModel::AtcCodeList), dlg.getSelectedCodes().join(";"));
                d->proxyModel->setData(d->proxyModel->index(index.row(), ComponentAtcModel::IsReviewed), 1);
            }
        } else if (selected == cnamts) {
            QDesktopServices::openUrl(QUrl(QString("http://www.codage.ext.cnamts.fr/codif/bdm//critere/index_lis_rech.php?p_nom_rech=%1%2&p_remb=tout&p_cri=subact&p_site=").arg("%25").arg(d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString().toUpper())));
        } else if (selected == google) {
            QDesktopServices::openUrl(QUrl(QString("http://www.google.fr/search?rls=en&q=%1+atc&ie=UTF-8&oe=UTF-8&redir_esc=").arg(d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString())));
        } else if (selected == who) {
            QDesktopServices::openUrl(QUrl(QString("http://www.whocc.no/atc_ddd_index/?name=%1").arg(d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString())));
        } else if (selected == resip) {
            QApplication::clipboard()->setText(d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString());
            QDesktopServices::openUrl(QUrl("http://www.portailmedicaments.resip.fr/bcb_recherche/classes.asp?cc=1"));
        } else if (selected == copyClip) {
            QApplication::clipboard()->setText(d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString());
        }
    }
}

void ComponentAtcEditorWidget::onComponentViewItemActivated(const QModelIndex &index)
{
    if (!d->proxyModel)
        return;
    if (index.column() == ComponentAtcModel::Name) {
        SearchAtcInDatabaseDialog dlg(this, d->proxyModel->index(index.row(), ComponentAtcModel::Name).data().toString());
        if (dlg.exec() == QDialog::Accepted) {
            d->proxyModel->setData(d->proxyModel->index(index.row(), ComponentAtcModel::AtcCodeList), dlg.getSelectedCodes().join(";"));
            d->proxyModel->setData(d->proxyModel->index(index.row(), ComponentAtcModel::IsReviewed), Qt::Checked, Qt::CheckStateRole);
        }
    }
}

namespace {
struct Unreviewed {
    Unreviewed(const QString &_name, const QStringList &_suggested, const QString &_comment):
        name(_name), comment(_comment), suggested(_suggested)
    {}

    // Sort by component name
    static bool lessThan(const Unreviewed &r1, const Unreviewed &r2)
    {
        return r1.name < r2.name;
    }

    QString name, comment;
    QStringList suggested;
};

const char * const WIKI_NOTE = \
        "===== Notes for user edition =====\n\n"
        "  * You can help the FreeMedForms project to improve the drugs database quality.\n"
        "  * The file (or these files) contains all the ATC-unlinked drugs component.\n"
        "  * The drug interaction engines require drug component to be linked to an ATC code."
        " Some codes are fake and created by the FreeDDIManager application. If you think we should "
        " create a fake ATC for a specific component please note it <code>**Fake ATC required**</code>"
        "\n"
        "  * If you edit this file, mark all your changes in **bold** <code>**my changes**</code>"
        "    or __underlined__ <code>__my changes__</code>\n"
        "  * You can also create a new line and sign it <code>  * Your text then click on sign</code>\n"
        "  * If the component does not have ATC code just mark: <code>**No ATC**</code>\n"
        "\n";

static void createNewWikiContent(const QStringList &dbUids, QMap<QChar, QString> &wikis, QString &wiki, QChar &current, int &nbCurrent)
{
    wiki.prepend(QString("~~NOCACHE~~ \n"
                         "====== Unreviewed components: %1 - Letter %2 ======\n\n\n"
                         "  * Number of unreviewed components: %3\n"
                         "  * Date of report generation : %4"
                         "\n\n"
                         "%5"
                         "\n\n")
                 .arg(dbUids.join("; "))
                 .arg(current)
                 .arg(nbCurrent)
                 .arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::LongFormat))
                 .arg(WIKI_NOTE)
                 );
    wikis.insert(current, wiki);

    // Start a new wiki content
    nbCurrent = 0;
    wiki.clear();
}

}

/**
 * Create a dokuwiki txt file with all information about unreviewed components.
 */
void ComponentAtcEditorWidget::onCreateUnreviewedFileRequested()
{
    // TODO: this code should be moved into DrugsDb::IDrugDatabase

    // Get all unreviewed component names with their comments and suggested ATC codes
    QList<Unreviewed> unrev;
    ComponentAtcModel *model = ddiCore()->componentAtcModel();
    for(int i = 0; i < model->rowCount(); ++i) {
        QModelIndex isRev = model->index(i, ComponentAtcModel::IsReviewed);
        bool reviewed = (model->data(isRev).toString().compare("Reviewed") == 0);
        // Reviewed component -> skip
        if (reviewed)
            continue;

        // Unreviewed component
        QModelIndex name = model->index(i, ComponentAtcModel::Name);
        if (name.data().toString().simplified().isEmpty())
            continue;
        QModelIndex atc = model->index(i, ComponentAtcModel::AtcCodeList);
        QModelIndex sugg = model->index(i, ComponentAtcModel::SuggestedAtcCodeList);
        QModelIndex comment = model->index(i, ComponentAtcModel::Comments);
        QStringList codes;
        codes << sugg.data().toString().split(";", QString::SkipEmptyParts);
        codes << atc.data().toString().split(";", QString::SkipEmptyParts);
        codes.removeDuplicates();
        codes.removeAll("");
        unrev << Unreviewed(name.data().toString(), codes, comment.data().toString());

        // TODO: How to add all drug brandname containing the component (using DrugsDb but we are in DDI)
    }
    qSort(unrev.begin(), unrev.end(), Unreviewed::lessThan);

    // Create a wiki like file content
    QString toc;
    toc += QString("====== Unreviewed components: %1 ======\n\n\n"
                    "  * Number of components: %2\n"
                    "  * Number of unreviewed components: %3\n"
                    "  * Date of report generation : %4"
                    "\n\n"
                    "%5"
                    "\n\n"
                   "===== Table of contents =====\n"
                   "\n\n")
            .arg(model->databaseUids().join("; "))
            .arg(model->rowCount())
            .arg(unrev.count())
            .arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::LongFormat))
            .arg(WIKI_NOTE);

    QMap<QChar, QString> wikis;
    QChar current;
    int nbCurrent = 0;
    QString wiki;
    foreach(const Unreviewed &ur, unrev) {
        QString normalizedName = Utils::removeAccents(ur.name);
        if (current.isNull())
            if (!normalizedName[0].isLetter())
                current = '0';
            else
                current = normalizedName[0];
        else if (current != normalizedName[0]) {
            // All numbers and [ ( stay in the same block
            if (normalizedName[0].isLetter()) {
                // New char -> Store the current wiki content
                createNewWikiContent(model->databaseUids(), wikis, wiki, current, nbCurrent);
                current = normalizedName[0];
            }
        }
        ++nbCurrent;

        // Create ATC links
        QString atc;
        foreach(const QString &code, ur.suggested)
            atc += QString("\n    * [[http://www.whocc.no/atc_ddd_index/?code=%1|%1]]").arg(code);

        // Create component search links
        // TODO: improve this for foreign drug database (we don't need CNAMTS links)
        QString search;
        search += QString("\n    * [[%1|Search CNAMTS database]]").arg(QString("http://www.codage.ext.cnamts.fr/codif/bdm//critere/index_lis_rech.php?p_nom_rech=%1%2&p_remb=tout&p_cri=subact&p_site=").arg("%25").arg(Utils::removeAccents(ur.name.toUpper()).simplified()));
        search += QString("\n    * [[%1|Search Google]]").arg(QString("http://www.google.fr/search?rls=en&q=%1+atc&ie=UTF-8&oe=UTF-8&redir_esc=").arg(ur.name.toUpper()));
        search += QString("\n    * [[%1|Search WHO ATC index]]").arg(QString("http://www.whocc.no/atc_ddd_index/?name=%1").arg(Utils::removeAccents(ur.name.toUpper())));
        search += QString("\n    * [[%1|Search RESIP]]").arg(QString("http://www.portailmedicaments.resip.fr/bcb_recherche/classes.asp?cc=1"));
        search += QString("\n    * [[%1|Search WikiPedia (En)]]").arg(QString("http://en.wikipedia.org/wiki/%1").arg(Utils::firstLetterUpperCase(Utils::removeAccents(ur.name.toLower()))));
        search += QString("\n    * [[%1|Search vidal.fr]]").arg(QString("http://www.vidal.fr/recherche/index/q:%1/").arg(Utils::removeAccents(ur.name.toLower())));

        wiki += QString("===== %1 =====\n\n"
                        "  * Your proposal: \n"
                        "  * Suggested ATC codes: %2\n"
                        "  * Comment: %3\n"
                        "  * Suggested search links: %4\n"
                        "\n\n")
                .arg(ur.name)
                .arg(atc)
                .arg(ur.comment)
                .arg(search)
                ;
    }
    createNewWikiContent(model->databaseUids(), wikis, wiki, current, nbCurrent);

    // Ask for a dir
    QString path = QFileDialog::getExistingDirectory(this,
                                            tr("Select a dir to save wiki content"),
                                            settings()->path(Core::ISettings::UserDocumentsPath));
    if (path.isEmpty())
        return;

    // Save all wiki content and create a TOC
    QMapIterator<QChar, QString> it(wikis);
    while (it.hasNext()) {
        it.next();
        // Save wiki content
        QString fileName = QString("%1/component_%2.txt").arg(path).arg(it.key().toLower());
        if (!Utils::saveStringToFile(it.value(), fileName, Utils::Overwrite, Utils::DontWarnUser)) {
            Utils::warningMessageBox(tkTr(Trans::Constants::FILE_1_CAN_NOT_BE_CREATED).arg(fileName), "");
            continue;
        }

        // Add a line to the TOC
        QString wikiUrl = QFileInfo(fileName).baseName();
        toc += QString("\n  * Component starting with [[%1|%2]]")
                .arg(wikiUrl)
                .arg(it.key());
    }
    QString fileName = QString("%1/toc.txt").arg(path);
    if (!Utils::saveStringToFile(toc,fileName, Utils::Overwrite, Utils::DontWarnUser))
            Utils::warningMessageBox(tkTr(Trans::Constants::FILE_1_CAN_NOT_BE_CREATED).arg(tr("Unreviewed component(s)")), "");

    Utils::informativeMessageBox(tr("Unreviewed component wiki report created."),
                                 tr("All files are stored in the following path:\n%1").arg(path));
}

/**
 * When model resets update the ui overview label
 */
void ComponentAtcEditorWidget::onModelReset()
{
    d->ui->overview->setText(d->model->overview());
}

/**
 * Manually submit the model
 */
void ComponentAtcEditorWidget::saveModel()
{
    d->model = ddiCore()->componentAtcModel();
    if (!d->model->submitAll())
        LOG_ERROR("Unable to submit model");
    onModelReset();
}

//struct StructMol {
//    QString name, atc, reference, comment;
//};

//void ComponentAtcEditorWidget::processCSVFile()
//{
//    // This code was used to retrieve Jim's molecules association.
//    // ***** Should not be reused *****
//    qWarning() << "processCSVFile";
//    QFile file(qApp->applicationDirPath() + Core::Constants::MACBUNDLE + "/../global_resources/sql/ca_atc_script_orphans_improved_tab.txt");
//    QString content;
//    file.open(QIODevice::ReadOnly | QIODevice::Text);
//    content = file.readAll();
////    content.replace("\t\n", "\t\t\n");
//    QStringList lines = content.split("\n");
//    qWarning() << lines.count() << content.count("\n");
//    QList<StructMol> mols;
//    foreach(const QString &line, lines) {
//        const QStringList &vals = line.split("\t");
//        StructMol mol;
//        if (vals.count() != 5) {
//            if (vals.count() >= 2) {
//                mol.name = vals.at(0).simplified();
//                mol.name.remove("\"");
//                mol.atc = vals.at(1);
//                mol.atc.remove("\"");
//                mol.atc.remove(" ");
//                mol.atc = mol.atc.simplified();
//                mols << mol;
//                continue;
//            } else {
//                qWarning() << vals;
//                continue;
//            }
//        }
//        mol.name = vals.at(0).simplified();
//        mol.name.remove("\"");
//        mol.atc = vals.at(1).simplified();
//        mol.atc.remove("\"");
//        mol.atc.remove(" ");
//        mol.atc = mol.atc.simplified();
//        mol.reference = vals.at(3).simplified();
//        mol.comment = vals.at(4).simplified();
//        mols << mol;
//    }
//    content.clear();
//    foreach(const StructMol &mol, mols) {
//        if (mol.atc.isEmpty())
//            content += "    <Molecule name=\""+ mol.name +"\" AtcCode=\"\" review=\"true\" reviewer=\"Jim Busser, MD (CA)\" reference=\"\" comment=\"No ATC Code found\" dateofreview=\""+QDate::currentDate().toString(Qt::ISODate)+"\"/>\n";
//        else
//            content += "    <Molecule name=\""+ mol.name +"\" AtcCode=\""+mol.atc+"\" review=\"true\" reviewer=\"Jim Busser, MD (CA)\" reference=\""+mol.reference+"\" comment=\""+mol.comment+"\" dateofreview=\""+QDate::currentDate().toString(Qt::ISODate)+"\"/>\n";
//    }
//    QFile out(qApp->applicationDirPath() + Core::Constants::MACBUNDLE + "/../global_resources/sql/mols.xml");
//    out.open(QFile::WriteOnly | QFile::Text);
//    out.write(content.toUtf8());
//    out.close();

//}

//void ComponentAtcEditorWidget::processCSVFile()
//{
//    // This code was used to retrieve Eric's molecules association.
//    // ***** Should not be reused *****
//    qWarning() << "processCSVFile";
//    QFile file(qApp->applicationDirPath() + Core::Constants::MACBUNDLE + "/../global_resources/sql/handmadeatclinks.csv");
//    QString content;
//    file.open(QIODevice::ReadOnly | QIODevice::Text);
//    content = file.readAll();
//    QStringList lines = content.split("\n");
//    qWarning() << lines.count() << content.count("\n");
//    QList<StructMol> mols;
//    foreach(const QString &line, lines) {
//        if (line.startsWith("//"))
//            continue;
//        QStringList vals = line.split(";");
//        StructMol mol;
//        if (vals.count() < 2) {
//            qWarning() << vals;
//            continue;
//        }
//        mol.name = vals.takeFirst().simplified().toUpper();
//        mol.name.remove("\"");
//        mol.atc = vals.join(",");
//        mols << mol;
//    }
//    content.clear();
//    foreach(const StructMol &mol, mols) {
//        if (mol.atc.isEmpty())
//            content += "    <Molecule name=\""+ mol.name +"\" AtcCode=\"\" review=\"true\" reviewer=\"Eric Maeker, MD (FR)\" reference=\"\" comment=\"No ATC Code found\" dateofreview=\""+QDate::currentDate().toString(Qt::ISODate)+"\"/>\n";
//        else
//            content += "    <Molecule name=\""+ mol.name +"\" AtcCode=\""+mol.atc+"\" review=\"true\" reviewer=\"Eric Maeker, MD (FR)\" reference=\""+mol.reference+"\" comment=\""+mol.comment+"\" dateofreview=\""+QDate::currentDate().toString(Qt::ISODate)+"\"/>\n";
//    }
//    QFile out(qApp->applicationDirPath() + Core::Constants::MACBUNDLE + "/../global_resources/sql/mols.xml");
//    out.open(QFile::WriteOnly | QFile::Text);
//    out.write(content.toUtf8());
//    out.close();
//}

void ComponentAtcEditorWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d->ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

#ifdef WITH_TESTS
void ComponentAtcEditorWidget::test_runAllTests()
{}
#endif