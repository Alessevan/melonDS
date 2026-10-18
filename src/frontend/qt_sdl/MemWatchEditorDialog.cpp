#include "MemWatchEditorDialog.h"

#include <QPushButton>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>

MemWatchEditor::MemWatchEditor(QWidget *parent, melonDS::NDS* nds)
    : QDialog(parent)
    , m_nds(nds)
    , m_posX(nullptr)
    , m_posY(nullptr)
    , m_posZ(nullptr)
    , m_velX(nullptr)
    , m_velY(nullptr)
    , m_velZ(nullptr)
    , m_state(nullptr)
    , m_param0(nullptr)
    , m_param1(nullptr)
    , m_param2(nullptr)
    , m_param3(nullptr)
    , m_read(nullptr)
    , m_save(nullptr)
{
    QFont monospaceFont("Courier", 9);
    setFont(monospaceFont);
    setMinimumHeight(700);
    setMinimumWidth(600);

    auto mainLayout = new QVBoxLayout(this);

    auto grid = new QGridLayout();

    int row = 0;
    grid->addWidget(new QLabel(tr("Actor Address (hex):")), row, 0);
    m_addressLabel = new QLabel(tr(""), this);
    grid->addWidget(m_addressLabel, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Actor ID:")), row, 0);
    m_idLabel = new QLabel(tr(""), this);
    grid->addWidget(m_idLabel, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Pos X:")), row, 0);
    m_posX = new QLineEdit(this);
    grid->addWidget(m_posX, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Pos Y:")), row, 0);
    m_posY = new QLineEdit(this);
    grid->addWidget(m_posY, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Pos Z:")), row, 0);
    m_posZ = new QLineEdit(this);
    grid->addWidget(m_posZ, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Vel X:")), row, 0);
    m_velX = new QLineEdit(this);
    grid->addWidget(m_velX, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Vel Y:")), row, 0);
    m_velY = new QLineEdit(this);
    grid->addWidget(m_velY, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Vel Z:")), row, 0);
    m_velZ = new QLineEdit(this);
    grid->addWidget(m_velZ, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("State:")), row, 0);
    m_state = new QLineEdit(this);
    grid->addWidget(m_state, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Flags (bits 0..31):")), row, 0);
    QWidget *flagsWidget = new QWidget(this);
    auto flagsLayout = new QGridLayout(flagsWidget);
    flagsLayout->setSpacing(2);

    for (int i = 0; i < 32; ++i)
    {
        QCheckBox *cb = new QCheckBox(QString::fromUtf8(""), this);
        m_flagChecks[i] = cb;
        int r = i / 8;
        int c = i % 8;
        flagsLayout->addWidget(cb, r, c);
    }
    grid->addWidget(flagsWidget, row, 1);

    row++;
    grid->addWidget(new QLabel(tr("Params 0..3:")), row, 0);
    auto pbox = new QWidget(this);
    auto ph = new QHBoxLayout(pbox);
    m_param0 = new QLineEdit(this);
    m_param1 = new QLineEdit(this);
    m_param2 = new QLineEdit(this);
    m_param3 = new QLineEdit(this);
    ph->addWidget(m_param0);
    ph->addWidget(m_param1);
    ph->addWidget(m_param2);
    ph->addWidget(m_param3);
    grid->addWidget(pbox, row, 1);

    mainLayout->addLayout(grid);

    auto btns = new QHBoxLayout();
    m_save = new QPushButton(tr("Save"), this);
    btns->addWidget(m_save);
    mainLayout->addLayout(btns);

    connect(m_save, &QPushButton::clicked, this, &MemWatchEditor::onSaveClicked);
}

void MemWatchEditor::closeEvent(QCloseEvent* event)
{
    QDialog::closeEvent(event);
}

MemWatchEditor::~MemWatchEditor()
{
}

bool MemWatchEditor::writeMemory(uint32_t address, const void* data, int size)
{
    if (!m_nds)
        return false;

    void *ptr = nullptr;
    melonDS::NDS *nds = m_nds;

    if (address < nds->ARM9.ITCMSize)
    {
        ptr = &nds->ARM9.ITCM[address & (melonDS::ITCMPhysicalSize - 1)];
    }
    else if (nds->ARM9.DTCM != nullptr && (address & nds->ARM9.DTCMMask) == nds->ARM9.DTCMBase)
    {
        ptr = &nds->ARM9.DTCM[address & (melonDS::DTCMPhysicalSize - 1)];
    }
    else if ((address & 0xFF000000) == 0x02000000)
    {
        if (nds->MainRAM != nullptr)
            ptr = &nds->MainRAM[address & nds->MainRAMMask];
    }
    else if ((address & 0xFF000000) == 0x03000000)
    {
        if (nds->SWRAM_ARM9.Mem != nullptr)
            ptr = &nds->SWRAM_ARM9.Mem[address & nds->SWRAM_ARM9.Mask];
    }

    if (!ptr)
        return false;

    if (size == 4)
        *(uint32_t *)ptr = *(const uint32_t *)data;
    else if (size == 2)
        *(uint16_t *)ptr = *(const uint16_t *)data;
    else if (size == 1)
        *(uint8_t *)ptr = *(const uint8_t *)data;
    else
        memcpy(ptr, data, size);

    return true;
}

MemWatchEditorActor* MemWatchEditorActor::currentDlg = nullptr;

MemWatchEditorActor::MemWatchEditorActor(QWidget* parent, melonDS::NDS* nds, ActorData actor) :
    MemWatchEditor(parent, nds)
    , m_actorData(actor)
{
    this->m_addressLabel->setText(QString::number(actor.address, 16));

    this->m_posX->setText(QString::number(actor.posX, 16));
    this->m_posY->setText(QString::number(actor.posY, 16));
    this->m_posZ->setText(QString::number(actor.posZ, 16));

    this->m_velX->setText(QString::number(actor.velX, 16));
    this->m_velY->setText(QString::number(actor.velY, 16));
    this->m_velZ->setText(QString::number(actor.velZ, 16));

    this->m_idLabel->setText(actor.actorIdString);
    this->m_state->setText(QString::number(actor.state, 16));

    const QString flagLabels[32] = {
        QString::fromUtf8("Alive"),
        QString::fromUtf8("Visible"),
        QString::fromUtf8("2"),
        QString::fromUtf8("Active"),
        QString::fromUtf8("4"),
        QString::fromUtf8("5"),
        QString::fromUtf8("6"),
        QString::fromUtf8("7"),
        QString::fromUtf8("Grabbed"),
        QString::fromUtf8("9"),
        QString::fromUtf8("Interacting"),
        QString::fromUtf8("11"),
        QString::fromUtf8("12"),
        QString::fromUtf8("13"),
        QString::fromUtf8("14"),
        QString::fromUtf8("15"),
        QString::fromUtf8("16"),
        QString::fromUtf8("17"),
        QString::fromUtf8("18"),
        QString::fromUtf8("19"),
        QString::fromUtf8("20"),
        QString::fromUtf8("21"),
        QString::fromUtf8("22"),
        QString::fromUtf8("23"),
        QString::fromUtf8("24"),
        QString::fromUtf8("25"),
        QString::fromUtf8("26"),
        QString::fromUtf8("27"),
        QString::fromUtf8("28"),
        QString::fromUtf8("29"),
        QString::fromUtf8("30"),
        QString::fromUtf8("31")
    };

    for (int i = 0; i < 32; ++i)
    {
        bool v = (actor.flags >> i) & 0x1;
        if (m_flagChecks[i])
        {
            m_flagChecks[i]->setText(flagLabels[i]);
            m_flagChecks[i]->setChecked(v);
        }
    }

    this->m_param0->setText(QString::number(actor.params[0], 16));
    this->m_param1->setText(QString::number(actor.params[1], 16));
    this->m_param2->setText(QString::number(actor.params[2], 16));
    this->m_param3->setText(QString::number(actor.params[3], 16));
}

MemWatchEditorActor::~MemWatchEditorActor()
{
    if (currentDlg == this)
        currentDlg = nullptr;
}

void MemWatchEditorActor::closeEvent(QCloseEvent* event)
{
    currentDlg = nullptr;
    QDialog::closeEvent(event);
}

void MemWatchEditorActor::onSaveClicked()
{
    if (!m_actorData.address)
        return;

    int32_t pos[3] = { m_posX->text().toInt(nullptr, 16), m_posY->text().toInt(nullptr, 16), m_posZ->text().toInt(nullptr, 16) };
    writeMemory(m_actorData.address + 0x04, pos, sizeof(pos));

    int32_t vel[3] = { m_velX->text().toInt(nullptr, 16), m_velY->text().toInt(nullptr, 16), m_velZ->text().toInt(nullptr, 16) };
    writeMemory(m_actorData.address + 0x1c, vel, sizeof(vel));

    int16_t s = (int16_t)m_state->text().toUInt(nullptr, 16);
    writeMemory(m_actorData.address + 0x4C, &s, sizeof(s));

    uint32_t flags = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (m_flagChecks[i] && m_flagChecks[i]->isChecked())
            flags |= (1u << i);
    }
    writeMemory(m_actorData.address + 0x58, &flags, sizeof(flags));

    uint32_t p0 = m_param0->text().toUInt(nullptr, 16);
    uint32_t p1 = m_param1->text().toUInt(nullptr, 16);
    uint32_t p2 = m_param2->text().toUInt(nullptr, 16);
    uint32_t p3 = m_param3->text().toUInt(nullptr, 16);
    writeMemory(m_actorData.address + 0x6C + 0*4, &p0, 4);
    writeMemory(m_actorData.address + 0x6C + 1*4, &p1, 4);
    writeMemory(m_actorData.address + 0x6C + 2*4, &p2, 4);
    writeMemory(m_actorData.address + 0x6C + 3*4, &p3, 4);
}

MemWatchEditorMapObject* MemWatchEditorMapObject::currentDlg = nullptr;

MemWatchEditorMapObject::MemWatchEditorMapObject(QWidget* parent, melonDS::NDS* nds, MapObjectData mapObjectData) :
    MemWatchEditor(parent, nds)
    , m_mapObjectData(mapObjectData)
{
    this->m_addressLabel->setText(QString::number(mapObjectData.address, 16));

    this->m_posX->setText(QString::number(mapObjectData.posX, 16));
    this->m_posY->setText(QString::number(mapObjectData.posY, 16));
    this->m_posZ->setText(QString::number(mapObjectData.posZ, 16));

    this->m_idLabel->setText(mapObjectData.mapObjectIdString);
    this->m_state->setText(QString::number(mapObjectData.state, 16));

    const QString flagLabels[32] = {
        QString::fromUtf8("Alive"),
        QString::fromUtf8("1"),
        QString::fromUtf8("2"),
        QString::fromUtf8("3"),
        QString::fromUtf8("4"),
        QString::fromUtf8("5"),
        QString::fromUtf8("6"),
        QString::fromUtf8("7"),
        QString::fromUtf8("8"),
        QString::fromUtf8("9"),
        QString::fromUtf8("10"),
        QString::fromUtf8("11"),
        QString::fromUtf8("12"),
        QString::fromUtf8("13"),
        QString::fromUtf8("14"),
        QString::fromUtf8("15"),
        QString::fromUtf8("16"),
        QString::fromUtf8("17"),
        QString::fromUtf8("18"),
        QString::fromUtf8("19"),
        QString::fromUtf8("20"),
        QString::fromUtf8("21"),
        QString::fromUtf8("22"),
        QString::fromUtf8("23"),
        QString::fromUtf8("24"),
        QString::fromUtf8("25"),
        QString::fromUtf8("26"),
        QString::fromUtf8("27"),
        QString::fromUtf8("28"),
        QString::fromUtf8("29"),
        QString::fromUtf8("30"),
        QString::fromUtf8("31")
    };

    for (int i = 0; i < 32; ++i)
    {
        bool v = (mapObjectData.flags >> i) & 0x1;
        if (m_flagChecks[i])
        {
            m_flagChecks[i]->setText(flagLabels[i]);
            m_flagChecks[i]->setChecked(v);
        }
    }

    this->m_param0->setText(QString::number(mapObjectData.params[0], 16));
    this->m_param1->setText(QString::number(mapObjectData.params[1], 16));
    this->m_param2->setText(QString::number(mapObjectData.params[2], 16));
    this->m_param3->setText(QString::number(mapObjectData.params[3], 16));
}

MemWatchEditorMapObject::~MemWatchEditorMapObject()
{
    if (currentDlg == this)
        currentDlg = nullptr;
}

void MemWatchEditorMapObject::closeEvent(QCloseEvent* event)
{
    currentDlg = nullptr;
    QDialog::closeEvent(event);
}

void MemWatchEditorMapObject::onSaveClicked()
{
    if (!m_mapObjectData.address)
        return;

    int32_t pos[3] = { m_posX->text().toInt(nullptr, 16), m_posY->text().toInt(nullptr, 16), m_posZ->text().toInt(nullptr, 16) };
    writeMemory(m_mapObjectData.address + 0x04, pos, sizeof(pos));


    int16_t s = (int16_t)m_state->text().toUInt(nullptr, 16);
    writeMemory(m_mapObjectData.address + 0x16, &s, sizeof(s));

    uint32_t flags = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (m_flagChecks[i] && m_flagChecks[i]->isChecked())
            flags |= (1u << i);
    }
    writeMemory(m_mapObjectData.address + 0x1C, &flags, sizeof(flags));

    uint32_t p0 = m_param0->text().toUInt(nullptr, 16);
    uint32_t p1 = m_param1->text().toUInt(nullptr, 16);
    uint32_t p2 = m_param2->text().toUInt(nullptr, 16);
    uint32_t p3 = m_param3->text().toUInt(nullptr, 16);
    writeMemory(m_mapObjectData.address + 0x20 + 0*4, &p0, 4);
    writeMemory(m_mapObjectData.address + 0x20 + 1*4, &p1, 4);
    writeMemory(m_mapObjectData.address + 0x20 + 2*4, &p2, 4);
    writeMemory(m_mapObjectData.address + 0x20 + 3*4, &p3, 4);
}