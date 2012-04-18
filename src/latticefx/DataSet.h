
#ifndef __LATTICEFX_DATA_SET_H__
#define __LATTICEFX_DATA_SET_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/Preprocess.h>
#include <latticefx/RTPOperation.h>
#include <latticefx/Renderer.h>

#include <osg/ref_ptr>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <string>
#include <list>
#include <map>


/** ISSUE: Are time values doubles, signed/unsigned ints, or something else? */
typedef double TimeValue;


// Forwards
namespace osg {
    class Group;
}


namespace lfx {


/** \class DataSet DataSet.h <DataSet.h>
\brief Data set storage and interface.
\details Create an instance of this class for each data set you application
uses and renders. Each DataSet can contain 0 or more ChannelData objects, which
are arrays of data such as xyz vertex values or other scalar data. The data set
can also contain 0 or more RTPOperation objects, which are mask, filter, or
channel creation functions. An example is the MyMask RTPOperation.
*/
class LATTICEFX_EXPORT DataSet
{
public:
    DataSet();
    virtual ~DataSet();


    //
    // Data section
    //

    /** \brief Add a data channel to the ChannelDataList for a specific time value \c time. */
    void addChannel( const ChannelDataPtr channel, const TimeValue& time=(TimeValue)0. );

    /** \brief Get a named channel for a specific time value \c time.
    \returns NULL if the named channel doesn't exist at the specified time. */
    ChannelDataPtr getChannel( const std::string& name, const TimeValue& time=(TimeValue)0. );

    /** \overload ChannelDataPtr DataSet::getChannel( const std::string& name, const TimeValue& time ); */
    const ChannelDataPtr getChannel( const std::string& name, const TimeValue& time=(TimeValue)0. ) const;

    /** \brief Get all channels for a specific time value \c time.
    \details Returns all channels as a ChannelDataList. */
    ChannelDataList& getChannels( const TimeValue& time=(TimeValue)0. );

    /** \overload ChannelDataList& DataSet::getChannels( const TimeValue& time ); */
    const ChannelDataList& getChannels( const TimeValue& time=(TimeValue)0. ) const;



    //
    // Preprocessing & Caching section
    //



    //
    // Run-Time Operations section
    //

    /** \brief Add a runtime processing operation to the end of the operation list.
    \details All runtime processing operations are executed sequentially. Note that
    the app is responsible for maintaining operation list order.*/
    void addOperation( const RTPOperationPtr op );

    /** \brief Insert a runtime processing operation at the specified index.
    \details RTPOperation objects already at location >= \c index will have their
    location indices incremented. */
    void insertOperation( const unsigned int index, const RTPOperationPtr op );

    /** \brief Insert \c op before \c location.
    \details \c location and other downstream RTPOperation objects will follow
    \c op during processing. */
    void insertOperation( const RTPOperationPtr location, const RTPOperationPtr op );

    /** \brief Return the total number of runtime processing operations. */
    unsigned int getNumOperations() const;

    /** \brief Get the operation at the specific index.
    \returns NULL if \c index >= getNumOperations(). */
    RTPOperationPtr getOperation( const unsigned int index );

    /** \overload RTPOperationPtr DataSet::getOperation( const unsigned int index ); */
    const RTPOperationPtr getOperation( const unsigned int index ) const;

    /** \brief Get all operations.
    \returns An empty list if no operations exist for this DataSet. */
    RTPOperationList& getOperations();

    /** \overload RTPOperationList& DataSet::getOprtations(); */
    const RTPOperationList& getOperations() const;

    /** Need to be able to set a list of operations.
    Store multiple lists and switch between them. */



    //
    // Rendering Framework
    //

    /** \brief Specify the Renderer plugin operation for creating the scene graph.
    \details TBD
    */
    void setRenderer( const RendererPtr renderer );

    /** \brief Get the Renderer plugin operation.
    \details TBD */
    RendererPtr getRenderer();
    /** \override */
    const RendererPtr getRenderer() const;

    /** \brief Get the OSG scene graph. */
    osg::Node* getSceneData();

    /** \brief Execute all operations (process all data and create scene graph). */
    bool processChanges();

    /** \brief Return the current mask.
    \details The return value depends on when the function is called.
    If called from any Preprocess operation, the returned array will be filled with 'true'
    (all elements enabled).
    If called from any RTPOperation, the return value will reflect the processing of any
    preceding hpst-based RTPOperations of type Mask.
    If called from the Renderer (or after the data has been processed and the scene graph
    generated), the return value will reflect the accumulation of all host-based RTP
    operations of type Mask. */
    const ChannelDataPtr getMask() const;

    typedef enum {
        NOT_DIRTY        = ( 0 ),
        PREPROCESS_DIRTY = ( 0x1 << 0 ),
        RTP_DIRTY        = ( 0x1 << 1 ),
        RENDERER_DIRTY   = ( 0x1 << 2 ),
        ALL_DIRTY = ( PREPROCESS_DIRTY | RTP_DIRTY | RENDERER_DIRTY )
    } DirtyFlags;
    void setDirty( const DirtyFlags flags=ALL_DIRTY );
    DirtyFlags getDirty() const;

protected:
    bool checkAndResizeMask();

    typedef std::map< TimeValue, ChannelDataList > TimeSeriesData;

    TimeSeriesData _data;
    PreprocessList _preprocess;
    RTPOperationList _ops;
    RendererPtr _renderer;

    ChannelDataOSGArrayPtr _mask;

    osg::ref_ptr< osg::Group > _sceneGraph;
    DirtyFlags _dirtyFlags;
};

typedef boost::shared_ptr< DataSet > DataSetPtr;
typedef std::list< RTPOperationPtr > DataSetList;


// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif
