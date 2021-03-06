/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 /* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_EP_DISCOVERY_H
#define _TJ_EP_DISCOVERY_H

#include "epinternal.h"
#include "epconnection.h"
#include <TJScout/include/tjservice.h>

namespace tj {
	namespace ep {
		class EPRemoteState;
		
		struct EPStateChangeNotification {
			tj::shared::ref<EPRemoteState> remoteState;
		};
		
		class EP_EXPORTED EPRemoteState: public virtual tj::shared::Object, public EPState {
			public:
				EPRemoteState(tj::shared::ref<EPEndpoint> ep);
				virtual ~EPRemoteState();
				virtual tj::shared::ref<EPEndpoint> GetEndpoint();
				tj::shared::Listenable<EPStateChangeNotification> EventStateChanged;
			
			private:
				tj::shared::weak<EPEndpoint> _ep;
		};
		
		class EP_EXPORTED DiscoveryDefinition: public virtual tj::shared::Object, public tj::shared::Serializable {
			public:
				virtual ~DiscoveryDefinition();
				virtual tj::shared::String GetType() const;
				
			protected:
				DiscoveryDefinition(const tj::shared::String& type);
				tj::shared::String _type;
		};
		
		class EP_EXPORTED DiscoveryDefinitionFactory: public tj::shared::PrototypeBasedFactory<DiscoveryDefinition> {
			public:
				virtual ~DiscoveryDefinitionFactory();
				virtual tj::shared::ref<DiscoveryDefinition> Load(TiXmlElement* me);
				virtual void Save(tj::shared::strong<DiscoveryDefinition> c, TiXmlElement* me);
				
				static tj::shared::strong<DiscoveryDefinitionFactory> Instance();
				
			protected:
				DiscoveryDefinitionFactory();
				static tj::shared::ref<DiscoveryDefinitionFactory> _instance;
		};
		
		struct EP_EXPORTED DiscoveryNotification {
			DiscoveryNotification();
			DiscoveryNotification(const tj::shared::Timestamp& ts, tj::shared::ref<Connection> m, bool add, EPMediationLevel ml);
			
			tj::shared::Timestamp when;
			tj::shared::ref<Connection> connection;
			tj::shared::ref<EPEndpoint> endpoint;
			tj::shared::ref<EPRemoteState> remoteState;
			tj::shared::ref<tj::scout::Service> service;
			bool added;
			EPMediationLevel mediationLevel;
		};
		
		class EP_EXPORTED Discovery: public virtual tj::shared::Object {
			public:
				virtual ~Discovery();
				virtual void Create(tj::shared::strong<DiscoveryDefinition> def, const tj::shared::String& ownMagic) = 0;
				
				tj::shared::Listenable<DiscoveryNotification> EventDiscovered;
		};
		
		class EP_EXPORTED DiscoveryFactory: public virtual tj::shared::PrototypeBasedFactory< Discovery > {
			public:
				virtual ~DiscoveryFactory();
				virtual tj::shared::ref<Discovery> CreateFromDefinition(tj::shared::strong<DiscoveryDefinition> cd, const tj::shared::String& ownMagic = L"");
				static tj::shared::strong< DiscoveryFactory > Instance();
				
			protected:
				DiscoveryFactory();
				static tj::shared::ref< DiscoveryFactory > _instance;
		};
	}
}

#endif