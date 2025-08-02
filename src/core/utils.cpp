#include <syncwav/utils.h>
#include <syncwav/log.h>


namespace swav {
	SWAV_API std::vector<Device> getPlaybackDevices() {
		ma_context context;
		ma_context_init(NULL, 0, NULL, &context);

		ma_device_info* playbackInfo;
		ma_uint32 playbackCnt;

		log::i("Getting list of all playback devices");
		ma_context_get_devices(&context, &playbackInfo, &playbackCnt, NULL, NULL);
		log::i("got {} playback devices", playbackCnt);
		std::vector<Device> devices(playbackCnt);
		for (ma_uint32 i = 0; i < playbackCnt; i++) {
			devices[i].id = playbackInfo[i].id;
			devices[i].name = playbackInfo[i].name;
			devices[i].isDefault = playbackInfo[i].isDefault;
			devices[i].type = DeviceType::PLAYBACK;
		}

		ma_context_uninit(&context);
		return devices;
	}


	SWAV_API std::vector<Device> getCaptureDevices() {
		ma_context context;
		ma_context_init(NULL, 0, NULL, &context);

		ma_device_info* captureInfo;
		ma_uint32 captureCnt;

		log::i("Getting list of all capture devices");
		ma_context_get_devices(&context, NULL, NULL, &captureInfo, &captureCnt);
		log::i("got {} capture devices", captureCnt);
		std::vector<Device> devices(captureCnt);
		for (ma_uint32 i = 0; i < captureCnt; i++) {
			devices[i].id = captureInfo[i].id;
			devices[i].name = captureInfo[i].name;
			devices[i].isDefault = captureInfo[i].isDefault;
			devices[i].type = DeviceType::CAPTURE;
		}

		ma_context_uninit(&context);
		return devices;
	}

	SWAV_API std::vector<Device> getAllDevices() {
		ma_context context;
		ma_context_init(NULL, 0, NULL, &context);

		ma_device_info* captureInfo, * playbackInfo;
		ma_uint32 captureCnt, playbackCnt;

		log::i("Getting list of all the devices");

		ma_context_get_devices(&context, &playbackInfo, &playbackCnt, &captureInfo, &captureCnt);
		std::vector<Device> devices(captureCnt + playbackCnt);

		log::i("got playback devices count: {} and capture devices count: {}", playbackCnt, captureCnt);
		for (ma_uint32 i = 0; i < playbackCnt; i++) {
			devices[i].id = playbackInfo[i].id;
			devices[i].name = playbackInfo[i].name;
			devices[i].isDefault = playbackInfo[i].isDefault;
			devices[i].type = DeviceType::PLAYBACK;
		}
		for (ma_uint32 i = 0; i < captureCnt; i++) {
			ma_uint32 j = i + playbackCnt;
			devices[j].id = captureInfo[i].id;
			devices[j].name = captureInfo[i].name;
			devices[j].isDefault = captureInfo[i].isDefault;
			devices[j].type = DeviceType::CAPTURE;
		}
		ma_context_uninit(&context);
		return devices;
	}

}