#include "creatorThread.h"

////////////////////////////////////////////////////////////////////////////////
CreatorThread::CreatorThread(QObject *parent) :
    QThread(parent)
{
	_cancel = false;
}

////////////////////////////////////////////////////////////////////////////////
void CreatorThread::run()
{
	_cancel = false;

	if (_createVolume == NULL) return;

	try
	{
		_createVolume->create();
	}
	catch (std::exception ex)
	{
	}
}

////////////////////////////////////////////////////////////////////////////////
void CreatorThread::slotOnCancel()
{
	_cancel = true;
}

////////////////////////////////////////////////////////////////////////////////
bool CreatorThread::checkCancel()
{
	return _cancel;
}