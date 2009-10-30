#ifndef _TJ_EP_CONNECTION_H
#define _TJ_EP_CONNECTION_H

#include "internal/ep.h"
#include "ependpoint.h"
#include "epmessage.h"
#include "../../../TJNP/include/tjnetworkaddress.h"

namespace tj {
	namespace ep {
		enum Direction {
			DirectionNone = 0,
			DirectionInbound = 1,
			DirectionOutbound = 2,
			DirectionBoth = DirectionInbound | DirectionOutbound,
		};
		
		struct EP_EXPORTED MessageNotification {
			MessageNotification(const tj::shared::Timestamp& ts, tj::shared::strong<Message> m);
			
			tj::shared::Timestamp when;
			tj::shared::strong<Message> message;
		};
		
		class EP_EXPORTED ConnectionDefinition: public virtual tj::shared::Object, public virtual EPTransport {
			public:
				virtual ~ConnectionDefinition();
				virtual std::wstring GetType() const;
				virtual void SaveConnection(TiXmlElement* me) = 0;
				virtual void LoadConnection(TiXmlElement* me) = 0;
				
			protected:
				ConnectionDefinition(const std::wstring& type);
				std::wstring _type;
		};
		
		class EP_EXPORTED ConnectionDefinitionFactory: public tj::shared::PrototypeBasedFactory<ConnectionDefinition> {
			public:
				virtual ~ConnectionDefinitionFactory();
				virtual tj::shared::ref<ConnectionDefinition> Load(TiXmlElement* me);
				virtual void Save(tj::shared::strong<ConnectionDefinition> c, TiXmlElement* me);
				
				static tj::shared::strong<ConnectionDefinitionFactory> Instance();
				
			protected:
				ConnectionDefinitionFactory();
				static tj::shared::ref<ConnectionDefinitionFactory> _instance;
		};
		
		class EP_EXPORTED Connection: public virtual tj::shared::Object {
			public:
				virtual ~Connection();
				virtual void Create(tj::shared::strong<ConnectionDefinition> def, Direction d, tj::shared::ref<EPEndpoint> parent) = 0;
				virtual void CreateForTransport(tj::shared::strong< tj::ep::EPTransport > ept, const tj::np::NetworkAddress& address) = 0;
				virtual void Send(tj::shared::strong< Message > msg) = 0;
				
				tj::shared::Listenable<MessageNotification> EventMessageReceived;
		};
		
		class EP_EXPORTED ConnectionFactory: public virtual tj::shared::PrototypeBasedFactory< Connection > {
			public:
				virtual ~ConnectionFactory();
				virtual tj::shared::ref<Connection> CreateFromDefinition(tj::shared::strong<ConnectionDefinition> cd, Direction d, tj::shared::ref<EPEndpoint> parent);
				virtual tj::shared::ref<Connection> CreateForTransport(tj::shared::strong< tj::ep::EPTransport > cd, const tj::np::NetworkAddress& address);
				
				static tj::shared::strong< ConnectionFactory > Instance();
				
			protected:
				ConnectionFactory();
				static tj::shared::ref< ConnectionFactory > _instance;
		};
		
	}
}

#endif