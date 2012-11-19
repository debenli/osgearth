/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
* Copyright 2008-2012 Pelican Mapping
* http://osgearth.org
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include <osgEarthSim/KDISLiveEntityProvider>

#include <osgEarth/Notify>

using namespace osgEarth::Sim;

KDISLiveEntityProvider::KDISLiveEntityProvider(const std::string &ip, int port):
_done(false),
_ip( ip ),
_port( port )
{
}

KDISLiveEntityProvider::~KDISLiveEntityProvider()
{
    cancel();
}

void KDISLiveEntityProvider::run()
{                    
    Connection conn( _ip, _port);

    KOCTET cBuffer[MAX_PDU_SIZE]; // Somewhere to store the data we receive.

    //We can filter based on exercise ID using a single filter like this.
    PDU_Factory factory;
    //factory.AddFilter( new FactoryFilterExerciseID( 1 ) );


    while (!_done)
    {
        try
        {                  
            KINT32 i32Recv = 0;

            i32Recv = conn.Receive( cBuffer, MAX_PDU_SIZE );
            if (i32Recv > 0)
            {                      
                KDataStream s( cBuffer, i32Recv );

                //If the PDU didn't match our filters, then we 
                auto_ptr< Header > pHeader = factory.Decode( s );
                if (!pHeader.get())
                {                          
                    continue;
                }

                //Take owner ship of this PDU                      
                Entity_State_PDU* entityState = dynamic_cast<Entity_State_PDU*>(pHeader.get());
                if (entityState)
                {
                    //Take ownership of this pdu
                    pHeader.release();
                    onEntityStateChanged( entityState );
                }                      
            }                  
        }
        catch( std::exception & e )
        {
            OE_NOTICE << e.what() << std::endl;            
        } 
    }
}

int KDISLiveEntityProvider::cancel()
{
    if ( isRunning() )
    {
        _done = true;  

        while( isRunning() )
        {        
            OpenThreads::Thread::YieldCurrentThread();
        }
    }
    return 0;
}

void KDISLiveEntityProvider::startImplementation()
{
    startThread();
}

void KDISLiveEntityProvider::stopImplementation()
{
    cancel();
} 