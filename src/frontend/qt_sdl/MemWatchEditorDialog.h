#ifndef MEMWATCHEDITORDIALOG_H
#define MEMWATCHEDITORDIALOG_H
#include <QDialog>
#include <QFont>
#include <array>

#include "MemWatchExplorerDialog.h"
#include "types.h"
#include "NDS.h"

class QPushButton;
class QLineEdit;
class QLabel;
class QCloseEvent;
class QCheckBox;

class MemWatchEditor : public QDialog
{
    Q_OBJECT

public:
    explicit MemWatchEditor(QWidget* parent, melonDS::NDS* nds);
    ~MemWatchEditor();

protected:
    int row = 0;
    QLabel* m_addressLabel;
    QLineEdit* m_posX;
    QLineEdit* m_posY;
    QLineEdit* m_posZ;
    QLineEdit* m_velX;
    QLineEdit* m_velY;
    QLineEdit* m_velZ;
    QLineEdit* m_state;
    std::array<QCheckBox*, 32> m_flagChecks;
    QLineEdit* m_param0;
    QLineEdit* m_param1;
    QLineEdit* m_param2;
    QLineEdit* m_param3;
    QLabel* m_idLabel;

    QPushButton* m_read;
    QPushButton* m_save;

    void closeEvent(QCloseEvent* event) override;
    bool writeMemory(uint32_t address, const void* data, int size);

private slots:
    virtual void onSaveClicked() = 0;

private:
    melonDS::NDS* m_nds;
};

class MemWatchEditorActor : public MemWatchEditor
{
    Q_OBJECT

public:
    explicit MemWatchEditorActor(QWidget* parent, melonDS::NDS* nds, ActorData actor);
    ~MemWatchEditorActor();

    static MemWatchEditorActor* currentDlg;
    static MemWatchEditorActor* openDlg(QWidget* parent, melonDS::NDS *nds, ActorData actor)
    {
        if (currentDlg)
        {
            currentDlg->activateWindow();
            return currentDlg;
        }

        currentDlg = new MemWatchEditorActor(parent, nds, actor);
        currentDlg->show();
        return currentDlg;
    }

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    virtual void onSaveClicked();

private:
    ActorData m_actorData;
};

class MemWatchEditorMapObject : public MemWatchEditor
{
    Q_OBJECT

public:
    explicit MemWatchEditorMapObject(QWidget* parent, melonDS::NDS* nds, MapObjectData mapObject);
    ~MemWatchEditorMapObject();

    static MemWatchEditorMapObject* currentDlg;
    static MemWatchEditorMapObject* openDlg(QWidget* parent, melonDS::NDS *nds, MapObjectData mapObject)
    {
        if (currentDlg)
        {
            currentDlg->activateWindow();
            return currentDlg;
        }

        currentDlg = new MemWatchEditorMapObject(parent, nds, mapObject);
        currentDlg->show();
        return currentDlg;
    }

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    virtual void onSaveClicked();

private:
    MapObjectData m_mapObjectData;
};


#endif // MEMWATCHEDITORDIALOG_H
