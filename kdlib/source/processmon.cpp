#include "stdafx.h"

#include <map>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/atomic.hpp>

#include "processmon.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo;
typedef boost::shared_ptr<ProcessInfo>  ProcessInfoPtr;

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo {

public:

    ModulePtr getModule(MEMOFFSET_64  offset);
    void insertModule( ModulePtr& module);
    void removeModule(MEMOFFSET_64  offset );

    void insertBreakpoint(BREAKPOINT_ID bpId,  BreakpointPtr& breakpoint);
    BreakpointPtr removeBreakpoint(BREAKPOINT_ID bpId);
    unsigned long getNumberBreakpoints();
    BreakpointPtr getBreakpointByIndex(unsigned long index);
    BreakpointPtr getBreakpointById(BREAKPOINT_ID  bpid );

    DebugCallbackResult breakpointHit( MEMOFFSET_64 offset, BreakpointType breakpointType);

private:

    typedef std::map<MEMOFFSET_64, ModulePtr> ModuleMap;
    ModuleMap  m_moduleMap;
    boost::recursive_mutex  m_moduleLock;


    typedef std::map<BREAKPOINT_ID, BreakpointPtr>  BreakpointIdMap;
    BreakpointIdMap  m_breakpointMap;

    typedef std::map<MEMOFFSET_64, BreakpointPtr>  BreakpointOffsetMap;
    BreakpointOffsetMap  m_softBpOffsetMap;
    BreakpointOffsetMap  m_accessBpOffsetMap;

    boost::recursive_mutex  m_breakpointLock;
};

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitorImpl {

public:

    ProcessMonitorImpl() : m_bpUnique(0x80000000)
    {}

public:

    void processStart( PROCESS_DEBUG_ID id );
    void processStop( PROCESS_DEBUG_ID id );
    void processAllStop();
    unsigned int getNumberProcesses();

    void moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    void moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset );
    DebugCallbackResult breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType);
    unsigned long getNumberBreakpoints();

    ModulePtr getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id );
    void insertModule( ModulePtr& module, PROCESS_DEBUG_ID id );

    BREAKPOINT_ID insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id = -1 );
    void removeBreakpoint( BREAKPOINT_ID  bpid );
    BreakpointPtr getBreakpointByIndex(unsigned long index);
    BreakpointPtr getBreakpointById(BREAKPOINT_ID bpid);

private:

    ProcessInfoPtr  getProcess( PROCESS_DEBUG_ID id );

    boost::recursive_mutex  m_lock;

    typedef std::map<PROCESS_DEBUG_ID, ProcessInfoPtr>  ProcessMap;
    ProcessMap  m_processMap;

    boost::atomic<unsigned long long>  m_bpUnique;

};

std::auto_ptr<ProcessMonitorImpl>  g_procmon;

///////////////////////////////////////////////////////////////////////////////

class CurrentProcessGuard {

public:

    CurrentProcessGuard() :
        m_procId( getCurrentProcessId() )
        {}

    ~CurrentProcessGuard() {
        setCurrentProcessById(m_procId);
    }

private:

    PROCESS_DEBUG_ID  m_procId;
};

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::init()
{
    g_procmon.reset( new ProcessMonitorImpl );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::deinit()
{
    g_procmon.reset(0);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processStart( PROCESS_DEBUG_ID id )
{
    g_procmon->processStart(id);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processStop( PROCESS_DEBUG_ID id )
{
    g_procmon->processStop(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processAllStop()
{
    g_procmon->processAllStop();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ProcessMonitor::getNumberProcesses()
{
    return g_procmon->getNumberProcesses();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset )
{
    g_procmon->moduleLoad(id, offset);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset )
{
    g_procmon->moduleUnload(id, offset);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    return g_procmon->breakpointHit(id, offset, breakpointType);
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessMonitor::getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->getModule(offset,id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->insertModule(module, id);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID ProcessMonitor::insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->insertBreakpoint( breakpoint, id );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::removeBreakpoint( BREAKPOINT_ID  bpid )
{
    g_procmon->removeBreakpoint(bpid);
}

///////////////////////////////////////////////////////////////////////////////

unsigned long ProcessMonitor::getNumberBreakpoints()
{
    return g_procmon->getNumberBreakpoints();
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitor::getBreakpointByIndex(unsigned long index)
{
    return g_procmon->getBreakpointByIndex(index);
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitor::getBreakpointById(BREAKPOINT_ID bpid )
{
    return g_procmon->getBreakpointById(bpid);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processStart( PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  proc = ProcessInfoPtr( new ProcessInfo() );
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap[id] = proc;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processStop( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap.erase(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processAllStop()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap.clear();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ProcessMonitorImpl::getNumberProcesses()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    return m_processMap.size();
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessMonitorImpl::getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    ModulePtr  module;

    if ( processInfo )
        module = processInfo->getModule(offset);

    return module;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::moduleLoad( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
    {
        processInfo->removeModule( offset );
        loadModule(offset);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::moduleUnload( PROCESS_DEBUG_ID id, MEMOFFSET_64  offset )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
        processInfo->removeModule( offset );
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::breakpointHit( PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        return processInfo->breakpointHit( offset, breakpointType );

    return DebugCallbackNoChange;
}


///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        return processInfo->insertModule(module);
}

///////////////////////////////////////////////////////////////////////////////

BREAKPOINT_ID ProcessMonitorImpl::insertBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    BREAKPOINT_ID  bpId = -1;

    if ( processInfo )
    {
        bpId = ++m_bpUnique;
        processInfo->insertBreakpoint(bpId, breakpoint);
    }

    return bpId;
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::removeBreakpoint( BREAKPOINT_ID  bpid )
{
    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        BreakpointPtr  bp = it->second->removeBreakpoint(bpid);
        if ( bp )
        {
            if ( it->first != getCurrentProcessId() )
            {
                CurrentProcessGuard  g;
                setCurrentProcessById( it->first );
                bp->remove();
            }
            else
            {
                bp->remove();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

unsigned long ProcessMonitorImpl::getNumberBreakpoints()
{
    unsigned long  breakpointNumber = 0;

    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        breakpointNumber += it->second->getNumberBreakpoints();
    }

    return breakpointNumber;
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitorImpl::getBreakpointByIndex(unsigned long index)
{
    unsigned long  breakpointNumber = 0;

    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        breakpointNumber = it->second->getNumberBreakpoints();
        if ( breakpointNumber < index)
        {
            index -= breakpointNumber;
        }
        else
        {
            return it->second->getBreakpointByIndex(index);
        }
    }

    return BreakpointPtr();
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessMonitorImpl::getBreakpointById(BREAKPOINT_ID bpid)
{
    BreakpointPtr  foundBp;

    boost::recursive_mutex::scoped_lock l(m_lock);

    for ( ProcessMap::iterator  it = m_processMap.begin(); it != m_processMap.end(); ++it )
    {
        foundBp = it->second->getBreakpointById(bpid);
        if (foundBp)
            break;
    }

    return foundBp;
}

///////////////////////////////////////////////////////////////////////////////

ProcessInfoPtr ProcessMonitorImpl::getProcess( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock l(m_lock);

    ProcessMap::iterator  it  = m_processMap.find(id);

    if ( it != m_processMap.end() )
        return it->second;

    ProcessInfoPtr  proc = ProcessInfoPtr( new ProcessInfo() );
    m_processMap[id] = proc;

    return proc;
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessInfo::getModule(MEMOFFSET_64  offset)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);

    ModuleMap::iterator it = m_moduleMap.find(offset);

    if ( it != m_moduleMap.end() )
        return it->second;

    for ( ModuleMap::iterator it = m_moduleMap.begin(); it != m_moduleMap.end(); ++it )
    {
        if ( it->second->getBase() <= offset && offset < it->second->getEnd() )
        {
           return it->second;
        }
    }

    return ModulePtr();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::insertModule( ModulePtr& module)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    m_moduleMap[ module->getBase() ] = module;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::removeModule(MEMOFFSET_64  offset )
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    m_moduleMap.erase(offset);
}

///////////////////////////////////////////////////////////////////////////////

void  ProcessInfo::insertBreakpoint(BREAKPOINT_ID bpId,  BreakpointPtr& breakpoint)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);
    
    m_breakpointMap[bpId] = breakpoint;

    switch( breakpoint->getType() )
    {
    case SoftwareBreakpoint:
        m_softBpOffsetMap[breakpoint->getOffset()] = breakpoint;
        break;
    case DataAccessBreakpoint:
        m_accessBpOffsetMap[breakpoint->getOffset()] = breakpoint;
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////

BreakpointPtr  ProcessInfo::removeBreakpoint(BREAKPOINT_ID bpId)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointIdMap::iterator   it = m_breakpointMap.find(bpId);
    if (it == m_breakpointMap.end() )
        return BreakpointPtr();

    BreakpointPtr breakpoint = it->second;

    m_breakpointMap.erase(it);

    switch( breakpoint->getType() )
    {
    case SoftwareBreakpoint:
        m_softBpOffsetMap.erase( breakpoint->getOffset() );
        break;
    case DataAccessBreakpoint:
        m_accessBpOffsetMap.erase( breakpoint->getOffset() );
        break;
    }

    return breakpoint;
}


///////////////////////////////////////////////////////////////////////////////

unsigned long ProcessInfo::getNumberBreakpoints()
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    return m_breakpointMap.size();
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessInfo::getBreakpointByIndex(unsigned long index)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    if ( index >= m_breakpointMap.size() )
        return BreakpointPtr();

    BreakpointIdMap::iterator  it = m_breakpointMap.begin();
    std::advance( it, index );
    return it->second;
}

///////////////////////////////////////////////////////////////////////////////

BreakpointPtr ProcessInfo::getBreakpointById(BREAKPOINT_ID  bpid )
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointIdMap::iterator   it = m_breakpointMap.find(bpid);
    if (it == m_breakpointMap.end() )
        return BreakpointPtr();

    return it->second;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessInfo::breakpointHit( MEMOFFSET_64 offset, BreakpointType breakpointType)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);

    BreakpointPtr breakpoint;

    switch( breakpointType )
    {
    case SoftwareBreakpoint:
        breakpoint = m_softBpOffsetMap[offset];
        break;
    case DataAccessBreakpoint:
        breakpoint = m_accessBpOffsetMap[offset];
        break;
    }
    
    if ( !breakpoint )
        return DebugCallbackNoChange;
        
    BreakpointCallback*  callback = breakpoint->getCallback();
    if ( callback == 0 )
        return DebugCallbackBreak;

    return callback->onHit();
}

/////////////////////////////////////////////////////////////////////////////


} //namesapce kdlib

