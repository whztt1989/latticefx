#include "UtlSettings.h"

void UtlSettings::initComboBox( QSettings *pset, QComboBox *pCtl, const QString &name, const QVariant &def )
{
    QVariant v = pset->value( name, def );
    int i = pCtl->findData( v );
    if ( i >= 0 ) pCtl->setCurrentIndex( i );
}

void UtlSettings::initPlainText( QSettings *pset, QPlainTextEdit *pCtl, const QString &name, const QString &def )
{
    QVariant v = pset->value( name, def );
    pCtl->setPlainText( v.toString() );
}

void UtlSettings::initCheckBox( QSettings *pset, QCheckBox *pCtl, const QString &name, bool def )
{
    bool b = pset->value( name, QVariant(def) ).toBool();
    pCtl->setChecked( b );
}

void UtlSettings::initRadioBtn( QSettings *pset, QRadioButton *pCtl, const QString &name, bool def )
{
    bool b = pset->value(name, QVariant(def)).toBool();
    pCtl->setChecked( b );
}


void UtlSettings::saveComboBox( QSettings *pset, QComboBox *pCtl, const QString &name )
{
    int i = pCtl->currentIndex();
    QVariant v = pCtl->itemData( i );
    pset->setValue( name, v );
}

void UtlSettings::savePlainText( QSettings *pset, QPlainTextEdit *pCtl, const QString &name )
{
    QString s = pCtl->toPlainText();
    pset->setValue( name, s );
}

void UtlSettings::saveCheckBox( QSettings *pset, QCheckBox *pCtl, const QString &name )
{
    bool b = pCtl->isChecked();
    pset->setValue( name, QVariant(b) );
}

void UtlSettings::saveRadioBtn( QSettings *pset, QRadioButton *pCtl, const QString &name )
{
    bool b = pCtl->isChecked();
    pset->setValue( name, QVariant(b) );
}

int UtlSettings::getSelectedValueInt( QComboBox *pCtl )
{
	int i = pCtl->currentIndex();
	if ( i < 0 ) return 0;

	return pCtl->itemData( i ).toInt();
}

int UtlSettings::getCheckedItems( QListWidget *pCtl, std::vector<int> *pIndexs )
{
	int count = 0;

	for ( int i=0; i<pCtl->count(); i++ )
	{
		QListWidgetItem *pItem = pCtl->item( i );
		if ( pItem->checkState() == Qt::Checked )
		{
			pIndexs->push_back( i );
			count++;
		}
	}

	return count;
}
