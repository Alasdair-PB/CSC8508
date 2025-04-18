//
// Contributors: Max Bolton
//
#pragma once

#include "INetworkComponent.h"
#include "AudioListenerComponent.h"
#include "Camera.h"
#include <fmod.hpp>
#include <fmod_errors.h>


namespace NCL::CSC8508 {

	const size_t MAXFRAMESIZE = 960;
	struct EncodedAudioPacket : INetworkPacket {
		unsigned char encodedFrame[MAXFRAMESIZE];
		size_t packetSize;
		uint32_t historyStamp;

		EncodedAudioPacket() {
			type = Component_Event;
			historyStamp = 0;
			packetSubType = None;
			size = sizeof(EncodedAudioPacket) - sizeof(GamePacket);
		}
	};

	struct RecievedAudioPacket {
		uint32_t historyStamp;
		size_t packetSize;
		unsigned char* encodedFrame;
	};

	class NetworkedListenerComponent : public AudioListenerComponent, public INetworkComponent {

	public:
		NetworkedListenerComponent(GameObject& gameObject, PerspectiveCamera& camera, int objId, int ownId, int componId, int pFabId, bool clientOwned) :
			AudioListenerComponent(gameObject, camera), INetworkComponent(objId, ownId, componId, pFabId, clientOwned) {}

		~NetworkedListenerComponent() {
			if (clientOwned) {
				StopNetworkedEncodeThread();
				CloseEncoder(encoder);
				audioEngine->StopRecording();
			}
			else {
				CloseDecoder(decoder);
			}
		}


		void OnAwake() override {

			if (clientOwned) {
				// Initialise Encoding Pipeline
				InitMicSound();
				RecordMic();

				encoder = OpenEncoder();
				encodeThreadRunning = true;
				StartNetworkedEncodeThread();
			}
			else
				decoder = OpenDecoder();
		}

		void Update(float deltaTime) override {
			dt = deltaTime;

			if(clientOwned){
				Vector3 pos = transform->GetPosition();
				Vector3 forward = Vector3(fForward.x, fForward.y, fForward.z);
				AudioListenerComponent::Update(deltaTime);
			}
			else {
				for (int i = 0; i < bufferSize; i++) {
					if (audioPacketQueue[i] != nullptr) {
						UpdateNetworkedDecode();
					}
				}
			}

		}

		virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
			static std::unordered_set<std::type_index> derivedTypes = {
				typeid(NetworkedListenerComponent),
				typeid(AudioListenerComponent),
				typeid(AudioObject),
				typeid(IComponent),
				typeid(INetworkComponent)
			};
			return derivedTypes;
		}

		#pragma region Encode Packets

		std::pair<std::vector<unsigned char>, size_t> EncodeOpusFrame(std::vector<short>& pcmData) {
			std::vector<unsigned char> opusFrame;

			std::vector<unsigned char> tempBuffer(4096); // ensures buffer is large enough for any frame
			int frameSize = 960;
			size_t totalEncodeSize = 0;

			// Ensure PCM data is always a multiple of 960 samples
			if (pcmData.size() % frameSize != 0) {
				pcmData.resize((pcmData.size() / frameSize) * frameSize);  // Trim to nearest multiple of 960
			}

			for (size_t i = 0; i < pcmData.size(); i += frameSize) {
				int encodedBytes = opus_encode(encoder, pcmData.data() + i, frameSize, tempBuffer.data(), tempBuffer.size());

				// if encoding successful, append the encoded frame to the output buffer
				if (encodedBytes > 0) {
					opusFrame.insert(opusFrame.end(), tempBuffer.begin(), tempBuffer.begin() + encodedBytes);
					totalEncodeSize += encodedBytes;
					//std::cout << "[DEBUG] EncodeOpusFrame() Encoded " << encodedBytes << " bytes." << std::endl;
				}
				else {
					// std::cerr << "[ERROR] EncodeOpusFrame() Error Encoding PCM Data" << std::endl;
				}
			}
			return { opusFrame, totalEncodeSize };
		}

		std::pair<std::vector<unsigned char>, size_t> StreamEncodeMic() {
			static unsigned int lastPos = 0;
			unsigned int recordPos = 0;
			unsigned int frameSize = 960; // 20ms at 48kHz
			fSystem->getRecordPosition(0, &recordPos);


			unsigned int availableSamples;
			// Calculate the number of samples available to read
			if (recordPos < lastPos) {
				availableSamples = (sampleRate - lastPos) + recordPos;
			}
			else {
				availableSamples = recordPos - lastPos;
			}

			// Calculate the number of complete frames available
			unsigned int completeFrames = availableSamples / frameSize;
			if (completeFrames == 0) return std::make_pair(std::vector<unsigned char>(), -1);

			// Calculate the number of samples to process
			unsigned int samplesToProcess = completeFrames * frameSize;

			// Lock the sound to access the PCM data
			PCMBufferLock lock = LockSound(micInput, lastPos * sizeof(short), samplesToProcess * sizeof(short));
			if (lock.result != FMOD_OK) {
				std::cerr << "[ERROR] micInput lock: " << FMOD_ErrorString(lock.result) << std::endl;
				return std::make_pair(std::vector<unsigned char>(), 0);
			}

			// init pcmData vector
			std::vector<short> pcmData;

			// insert first chunk of data
			short* firstChunk = static_cast<short*>(lock.ptr1);
			pcmData.insert(pcmData.end(), firstChunk, firstChunk + (lock.len1 / sizeof(short)));

			// insert second chunk if it exists
			if (lock.ptr2 && lock.len2 > 0) {
				short* secndChunk = static_cast<short*>(lock.ptr2);
				pcmData.insert(pcmData.end(), secndChunk, secndChunk + (lock.len2 / sizeof(short)));
			}

			// Data has been copied, release lock
			UnlockSound(micInput, &lock);

			// Push samples to the ring buffer
			if (pcmData.size() > samplesToProcess) {
				pcmData.resize(samplesToProcess);
			}

			// Update last position
			lastPos = (lastPos + samplesToProcess) % sampleRate;

			std::pair<std::vector<unsigned char>, size_t> encodedPair = EncodeOpusFrame(pcmData);
			if (encodedPair.second > 0) {
				return encodedPair;
			}
			else {
				return std::make_pair(std::vector<unsigned char>(), -1);
			}

		}

		#pragma endregion

		#pragma region Send Audio Packets



		void SendEncodedAudioPacket(std::pair<std::vector<unsigned char>, size_t> encodedPair) {

			if (encodedPair.first.empty() || (encodedPair.second <= 0 || encodedPair.second > MAXFRAMESIZE)) {
				return;
			}

			EncodedAudioPacket* packet = new EncodedAudioPacket();


			std::copy(encodedPair.first.begin(), encodedPair.first.end(), packet->encodedFrame);

			packet->historyStamp = sendHistoryCounter;
			packet->packetSize = encodedPair.second;


			//std::cout << "Packet Size: " << packet->packetSize << "/" << MAXFRAMESIZE << std::endl;

			SendEventPacket(packet);
	
			delete packet;
			sendHistoryCounter++;
		}

		#pragma endregion

		#pragma region Recieve Audio Packets

		// Recieves an audio packet from the network of the same object id
		bool ReadEventPacket(INetworkPacket& p) override {
			if (p.packetSubType == None) {

				EncodedAudioPacket* encodedPacket = (EncodedAudioPacket*)&p;
				if (encodedPacket->encodedFrame == NULL) return false;

				size_t index = encodedPacket->historyStamp % bufferSize;

				RecievedAudioPacket* recieved = new RecievedAudioPacket();
				recieved->historyStamp = encodedPacket->historyStamp;
				recieved->packetSize = encodedPacket->packetSize;


				recieved->encodedFrame = new unsigned char[encodedPacket->packetSize];
				std::copy(encodedPacket->encodedFrame, encodedPacket->encodedFrame + encodedPacket->packetSize, recieved->encodedFrame);

				{
					std::lock_guard<std::mutex> lock(audioQueueMutex);
					audioPacketQueue[index] = recieved;
				}

				//std::cout << "Recieved Packet: " << recieved->historyStamp << "/" << recieveHistoryCounter << std::endl;
				return true;
			}
			else return false;
		}
		
		void UpdateNetworkedDecode() {

			uint32_t nextIndex = recieveHistoryCounter % bufferSize;

			//std::cout << "Next Index: " << nextIndex << "/" << bufferSize << std::endl;

			RecievedAudioPacket* packet = nullptr;

			{
				std::lock_guard<std::mutex> lock(audioQueueMutex);
				packet = audioPacketQueue[nextIndex];
			}

			// if no valid packet or packet is outdated, insert silence
			if (packet != nullptr && packet->historyStamp >= recieveHistoryCounter) {
				 // Decode valid packet and increment history counter
				DecodePersistentPlayback(audioPacketQueue[nextIndex]);
			}

			{
				std::lock_guard<std::mutex> lock(audioQueueMutex);
				audioPacketQueue[nextIndex] = nullptr;
			}

			recieveHistoryCounter++;
		}


		#pragma endregion

		#pragma region Decode Packets

		void SetPersistentSound(std::pair<FMOD::Sound*, FMOD::Channel*> persistentPair) {
			persistentSound = persistentPair.first;
			persistentChannel = persistentPair.second;
		}

		std::vector<short> DecodeOpusFrame(RecievedAudioPacket* encodedPacket) {
			std::vector<short> pcmFrame(960);

			unsigned char* packetData = encodedPacket->encodedFrame;
			size_t frameSize = encodedPacket->packetSize;

			int decodedSamples = opus_decode(decoder, packetData, frameSize, pcmFrame.data(), pcmFrame.size(), 0);


			if (decodedSamples < 0) {
				std::cerr << "[ERROR] UpdatePersistentPlayback() Opus Decoding Failed: " << decodedSamples << std::endl;
				return std::vector<short>();
			}

			if (decodedSamples < static_cast<int>(pcmFrame.size())) {
				pcmFrame.resize(decodedSamples);
			}

			return pcmFrame;

		}

		void DecodePersistentPlayback(RecievedAudioPacket* encodedPacket) {

			if (!persistentSound) return;

			std::vector<short> pcmFrame = DecodeOpusFrame(encodedPacket);

			if (pcmFrame.empty()) {
				std::cerr << "[ERROR] UpdatePersistentPlayback() Decoded PCM Frame is empty." << std::endl;
				return;
			}

			unsigned int bytesToWrite = pcmFrame.size() * sizeof(short);
			unsigned int bufferSizeBytes = persistentBufferSize * sizeof(short);

			if (currentWritePos * sizeof(short) + bytesToWrite > bufferSizeBytes) {

				unsigned int bytesUntilEnd = bufferSizeBytes - currentWritePos * sizeof(short);
				void* ptr1 = nullptr;
				void* ptr2 = nullptr;
				unsigned int len1 = 0, len2 = 0;

				FMOD_RESULT result = persistentSound->lock(currentWritePos * sizeof(short), bytesUntilEnd, &ptr1, &ptr2, &len1, &len2);

				if (result != FMOD_OK) {
					std::cerr << "[ERROR] UpdatePersistentPlayback() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
					return;
				}

				// Copy the first chunk of data
				memcpy(ptr1, pcmFrame.data(), bytesUntilEnd);
				persistentSound->unlock(ptr1, ptr2, len1, len2);

				// Copy the remaining data to the start of the buffer
				unsigned int remainingBytes = bytesToWrite - bytesUntilEnd;
				result = persistentSound->lock(0, remainingBytes, &ptr1, &ptr2, &len1, &len2);

				if (result != FMOD_OK) {
					std::cerr << "[ERROR] UpdatePersistentPlayback() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
					return;
				}

				memcpy(ptr1, ((char*)pcmFrame.data()) + bytesUntilEnd, remainingBytes);
				persistentSound->unlock(ptr1, ptr2, len1, len2);

				currentWritePos = remainingBytes / sizeof(short);

			}
			else { // normal write, no wrap-around
				void* ptr1 = nullptr;
				void* ptr2 = nullptr;
				unsigned int len1 = 0, len2 = 0;
				FMOD_RESULT result = persistentSound->lock(currentWritePos * sizeof(short), bytesToWrite, &ptr1, &ptr2, &len1, &len2);
				if (result != FMOD_OK) {
					std::cerr << "[ERROR] UpdatePersistentPlayback() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
					return;
				}
				memcpy(ptr1, pcmFrame.data(), bytesToWrite);
				persistentSound->unlock(ptr1, ptr2, len1, len2);
				currentWritePos = (currentWritePos + pcmFrame.size()) % persistentBufferSize;
			}

		}

		#pragma endregion

		#pragma region Encode Thread

		// Only start the encode thread if the object is owned by the client
		void StartNetworkedEncodeThread() {
			std::cout << "Networked Audio Encode Started" << std::endl;

			encodeThread = std::thread([this]() {
				// Set the update interval to 20ms
				auto nextUpdateTime = std::chrono::steady_clock::now();
				while (encodeThreadRunning && audioEngine->GetIsSystemValid()) {
					// Schedule the next update
					nextUpdateTime += std::chrono::milliseconds(20);

					// Call audio encoding if mic is recording
					if (audioEngine->IsRecording()) {
						SendEncodedAudioPacket(StreamEncodeMic());
					}

					// Sleep until the next scheduled update time
					std::this_thread::sleep_until(nextUpdateTime);
				}
				});
		}

		void StopNetworkedEncodeThread() {
			encodeThreadRunning = false;
			if (encodeThread.joinable()) {
				encodeThread.join();
				std::cout << "Networked Audio Encode Stopped" << std::endl;
			}
		}

		#pragma endregion

		

	private:
		uint32_t dt = 0;

		std::mutex audioQueueMutex;

	protected:

		/////////////Encoding///////////////
		OpusEncoder* encoder;
		bool encodeThreadRunning;
		std::thread encodeThread;

		std::deque<std::pair<std::vector<unsigned char>, size_t>> exportPacketQueue;

		// Send Data
		uint32_t sendHistoryCounter = 0;


		/////////////Decoding///////////////
		OpusDecoder* decoder;

		const size_t bufferSize = 64;
		std::vector<RecievedAudioPacket*> audioPacketQueue{ bufferSize, nullptr };

		// Recieve Data
		uint32_t recieveHistoryCounter = 0;

		FMOD::Sound* persistentSound = nullptr;
		FMOD::Channel* persistentChannel = nullptr;
		unsigned int persistentBufferSize = sampleRate / 2; // 0.5 second
		unsigned int currentWritePos = 0; // in samples
		/////////////////////////


	};
}