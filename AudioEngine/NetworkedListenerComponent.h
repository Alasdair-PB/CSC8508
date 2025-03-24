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
		NetworkedListenerComponent(GameObject& gameObject, PerspectiveCamera& camera, int objId, int ownId, int componId, bool clientOwned) :
			AudioListenerComponent(gameObject, camera), INetworkComponent(objId, ownId, componId, clientOwned) {}

		~NetworkedListenerComponent() = default;

		/*
		CloseEncoder(encoder);
		CloseDecoder(decoder);
		if (IsRecording()) {
			StopRecording();
		}
		if (persistentSound) {
			persistentSound->release();
		}
		*/


		void OnAwake() override {
		
			if (clientOwned) {
				// Initialise Encoding Pipeline
				InitMicSound();
				encoder = OpenEncoder();

				RecordMic();

				encodeThreadRunning = true;
				StartNetworkedEncodeThread();
			}

			decoder = OpenDecoder();
			decodeThreadRunning = true;
			StartNetworkedDecodeThread();

			std::cout << "Listener ID: " << objectID << ", awake!" << std::endl;
		}

		void Update(float deltaTime) override {
			dt = deltaTime;
			// Send packets as they are encoded
			//if(clientOwned)UpdatePacketSend();
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
			if (completeFrames == 0) return std::make_pair(std::vector<unsigned char>(), 0);

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
			if (!encodedPair.first.empty()) {
				return encodedPair;
			}
			else {
				return std::make_pair(std::vector<unsigned char>(), 0);
			}

		}

#pragma endregion

		#pragma region Send Audio Packets

		void UpdatePacketSend() {
			if (exportPacketQueue.empty()) return;

			//check if first packet is valid
			if (exportPacketQueue.front().first.empty()) {
				exportPacketQueue.pop_front();
				return;
			}

			SendEncodedAudioPacket(exportPacketQueue.front());
			std::cout << "Sent Packet ID: " << objectID << std::endl;
			exportPacketQueue.pop_front();
		}

		void SendEncodedAudioPacket(std::pair<std::vector<unsigned char>, size_t> encodedPair) {
			EncodedAudioPacket* packet = new EncodedAudioPacket();
			if (encodedPair.first.empty() or packet->packetSize <= 0) {
				delete packet;
				return;
			}

			std::copy(encodedPair.first.begin(), encodedPair.first.end(), packet->encodedFrame);

			packet->historyStamp = sendHistoryCounter;
			packet->packetSize = encodedPair.second;
			SendEventPacket(packet);
	
			delete packet;
			sendHistoryCounter++;
		}

		#pragma endregion

		#pragma region Recieve Audio Packets

		// Recieves an audio packet from the network of the same object id
		bool ReadEventPacket(INetworkPacket& p) override {
			if (p.packetSubType == None) {

				EncodedAudioPacket* packet = (EncodedAudioPacket*)&p;
				if (packet->encodedFrame == NULL) return false;


				{
					std::lock_guard<std::mutex> lock(audioQueueMutex);
					if (audioPacketQueue.empty()) { // if first packet set playbackStartTime to current time + jitterDelays
						playbackStartTime = dt + jitterDelayMS;
					}
				}

				size_t index = packet->historyStamp % bufferSize;

				RecievedAudioPacket* recieved = new RecievedAudioPacket();
				recieved->historyStamp = packet->historyStamp;
				recieved->packetSize = packet->packetSize;


				recieved->encodedFrame = new unsigned char[packet->packetSize];
				std::copy(packet->encodedFrame, packet->encodedFrame + packet->packetSize, recieved->encodedFrame);

				{
					std::lock_guard<std::mutex> lock(audioQueueMutex);
					audioPacketQueue[index] = recieved;
				}

				std::cout << "Recieved Packet ID: " << packet->ownerID << std::endl;
				return true;
			}
			else return false;
		}
		
		#pragma endregion

		#pragma region Decode Packets

		void SetPersistentSound(FMOD::Sound* sound) {
			persistentSound = sound;
		}

		void UpdateNetworkedDecode() {
			uint32_t nextIndex = recieveHistoryCounter % bufferSize;
			RecievedAudioPacket* packet = nullptr;

			{
				std::lock_guard<std::mutex> lock(audioQueueMutex);
				packet = audioPacketQueue[nextIndex];
			}


			if (packet == nullptr ||
				packet->historyStamp < recieveHistoryCounter) {
				std::cout << "Inserting Silence" << std::endl;
				//InsertSilence(nextIndex);
			}
			else {
				DecodePersistentPlayback(audioPacketQueue[nextIndex]);
				recieveHistoryCounter++;
			}

			{
				std::lock_guard<std::mutex> lock(audioQueueMutex);
				audioPacketQueue[nextIndex] = nullptr;
			}
		}
		

		/**
		* TODO: Insert silence into the audio queue
		*/
		void InsertSilence(size_t index) {

			RecievedAudioPacket* silentPacket = new RecievedAudioPacket();
			
			silentPacket->encodedFrame = new unsigned char[1];
			silentPacket->encodedFrame[0] = 0;

			silentPacket->historyStamp = recieveHistoryCounter;
			silentPacket->packetSize = 1;

			audioPacketQueue[index] = silentPacket;

		}


		std::vector<short> DecodeOpusFrame(RecievedAudioPacket* encodedPacket) {
			std::vector<short> pcmFrame(960);

			unsigned char* packetData = encodedPacket->encodedFrame;
			size_t frameSize = encodedPacket->packetSize;

			int decodedSamples = opus_decode(decoder, packetData, frameSize, pcmFrame.data(), pcmFrame.size(), 0);

			std::cout << "[DEBUG] UpdatePersistentPlayback() Decoded " << decodedSamples << " samples." << std::endl;

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

		#pragma region Threads

		// Only start the encode thread if the object is owned by the client
		void StartNetworkedEncodeThread() {
			std::cout << "Networked Audio Encode Started" << std::endl;

			encodeThread = std::thread([this]() {
				// Set the update interval to 20ms
				auto nextUpdateTime = std::chrono::steady_clock::now();
				while (encodeThreadRunning) {
					// Schedule the next update
					nextUpdateTime += std::chrono::milliseconds(20);

					// Call audio encoding if mic is recording
					if (IsRecording()) {
						SendEncodedAudioPacket(StreamEncodeMic());
					}

					// Sleep until the next scheduled update time
					std::this_thread::sleep_until(nextUpdateTime);
				}
				});
		}

		void StopNetworkedEncodeThread() {
			if (encodeThread.joinable()) {
				encodeThread.join();
				std::cout << "Networked Audio Encode Stopped" << std::endl;
			}
		}

		void StartNetworkedDecodeThread() {
			std::cout << "Networked Audio Decode Started" << std::endl;

			decodeThread = std::thread([this]() {
				while (decodeThreadRunning) {

					for (size_t i = 0; i < bufferSize; i++) {
						if (audioPacketQueue[i] != nullptr) {
							UpdateNetworkedDecode();
						}
					}
				}
			});
		}

		void StopNetworkedDecodeThread() {
			if (decodeThread.joinable()) {
				decodeThread.join();
				std::cout << "Networked Audio Decode Stopped" << std::endl;
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
		bool decodeThreadRunning;
		std::thread decodeThread;


		const size_t bufferSize = 64;
		std::vector<RecievedAudioPacket*> audioPacketQueue{ bufferSize, nullptr };

		// Recieve Data
		uint32_t recieveHistoryCounter = 0;

		FMOD::Sound* persistentSound = nullptr;
		unsigned int persistentBufferSize = sampleRate / 2; // 0.5 second
		unsigned int currentWritePos = 0; // in samples
		/////////////////////////



		// Playback Data
		uint32_t jitterDelayMS = 100;
		uint32_t playbackStartTime = 0;


	};
}