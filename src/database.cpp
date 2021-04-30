#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QFileDialog>

#include <passman/data_stream.hpp>
#include <passman/pdpp_entry.hpp>

#include "database.hpp"
#include "gui/password_widget.hpp"
#include "gui/password_generator_dialog.hpp"
#include "gui/entry_edit_widget.hpp"

// Add a new entry.
void Database::add() {
    passman::PDPPEntry *entry = new passman::PDPPEntry({}, this);
    addEntry(entry);

    EntryEditWidget *di = new EntryEditWidget(entry);
    di->setup();

    di->show();
}

int Database::saveAsPrompt() {
    const QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save Location"), "", passman::Constants::fileExt);
    return saveAs(fileName);
}

passman::KDF *Database::makeKdf(uint8_t t_hmac, uint8_t t_hash, uint8_t t_encryption, passman::VectorUnion t_seed, passman::VectorUnion t_keyFile, uint8_t t_hashIters, uint16_t t_memoryUsage)
{
    QVariantMap kdfMap({
        {"hmac", t_hmac == 63 ? hmac : t_hmac},
        {"hash", t_hash == 63 ? hash : t_hash},
        {"encryption", t_encryption == 63 ? encryption : t_encryption},
        {"seed", t_seed.empty() ? iv.asQByteArray() : t_seed.asQByteArray()},
        {"keyfile", t_keyFile.empty() ? keyFilePath.asQVariant() : t_keyFile.asQVariant()}
    });

    uint16_t iters = t_hashIters == 0 ? hashIters : t_hashIters;

    switch (t_hash == 63 ? hash : t_hash) {
        case 0: {
            kdfMap.insert({
                              {"i1", (t_memoryUsage == 0 ? memoryUsage : t_memoryUsage) * 1000},
                              {"i2", iters},
                              {"i3", 1}
                          });
            break;
        } case 2: {
            kdfMap.insert({
                              {"i1", 32768},
                              {"i2", iters},
                              {"i3", 1}
                          });
            break;
        } default: {
            kdfMap.insert("i1", iters);
            break;
        }
    }
    return new passman::KDF(kdfMap);
}
