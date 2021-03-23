#include <QDialogButtonBox>

#include "base_widget.hpp"
#include "../database.hpp"

BaseWidget::BaseWidget(Database *t_database)
    : database(t_database)
    , window(t_database->window)
    , buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
    , layout(new QGridLayout(this))
{}
