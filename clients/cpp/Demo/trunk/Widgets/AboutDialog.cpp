#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent)
{
    setupUi(this);
}

void AboutDialog::closeDialog()
{
    close();
}
