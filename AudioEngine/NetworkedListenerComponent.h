//
// Contributors: Max Bolton
//
#pragma once

#include "INetworkComponent.h"
#include "AudioListenerComponent.h"
#include "Camera.h"


namespace NCL::CSC8508 {

	struct EncodedAudioPacket : INetworkPacket {
		unsigned char* encodedFrame;
		uint32_t historyStamp;

		EncodedAudioPacket() {
			type = Component_Event;
			packetSubType = None;
			size = sizeof(EncodedAudioPacket) - sizeof(GamePacket);
		}
	};

	class NetworkedListenerComponent : public AudioListenerComponent, public INetworkComponent {

	public:
		NetworkedListenerComponent(GameObject& gameObject, PerspectiveCamera& camera, int objId, int ownId, int componId, bool clientOwned) :
			AudioListenerComponent(gameObject, camera), INetworkComponent(objId, ownId, componId, clientOwned), reset(true) {}

		~NetworkedListenerComponent() = default;

		void OnAwake() override {}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> derivedTypes = {
				typeid(NetworkedListenerComponent),
				typeid(AudioListenerComponent),
				typeid(INetworkComponent)
			};
			return derivedTypes;
		}

		void SendEncodedAudioPacket(unsigned char* encodedPacket) {
			EncodedAudioPacket* packet = new EncodedAudioPacket();
			packet->encodedFrame = encodedPacket;
			SendEventPacket(packet);
			delete packet;
		}

		// Send audio packets to the network
		void UpdateAudioPacket(float delatTime) {

			EncodedAudioPacket* audioPacket = new EncodedAudioPacket();

			audioPacket->historyStamp = lastHistoryEntry;
			audioPacket->encodedFrame = nullptr;

			lastHistoryEntry++;
		}

		// Recieves an audio packet from the network
		bool ReadEventPacket(INetworkPacket& p) override {
			if (p.packetSubType == None) {
				EncodedAudioPacket* packet = (EncodedAudioPacket*)&p;
				ReadAudioPacket audioPacket = ReadAudioPacket(packet->encodedFrame, packet->historyStamp);
				audioPacketQueue.push(audioPacket);
				return true;
			}
			else return false;
		}


	private:
		bool reset;
		int frameCount;
		int skippedFrames;
		const int skipLimit = 10;

	protected:

		struct ReadAudioPacket {
			unsigned char* encodedPacket;
			uint32_t historyStamp;


			ReadAudioPacket(unsigned char* encodedPacket, uint32_t historyStamp) {
				this->encodedPacket = encodedPacket;
				this->historyStamp = historyStamp;
			}
		};

		std::queue<ReadAudioPacket> audioPacketQueue;
		int lastHistoryEntry = 0;

	};
}