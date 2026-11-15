import 'package:file_picker/file_picker.dart';

import '../models/media_asset.dart';

/// Shared file-picker entry points for importing project media.
class MediaImportService {
  const MediaImportService();

  Future<List<MediaAsset>> pickVideos() =>
      _pick(FileType.video, MediaAssetType.video, allowMultiple: true);

  Future<List<MediaAsset>> pickAudio() =>
      _pick(FileType.audio, MediaAssetType.audio);

  Future<List<MediaAsset>> pickImages() =>
      _pick(FileType.image, MediaAssetType.image, allowMultiple: true);

  Future<List<MediaAsset>> _pick(
    FileType fileType,
    MediaAssetType assetType, {
    bool allowMultiple = false,
  }) async {
    final result = await FilePicker.platform.pickFiles(
      type: fileType,
      allowMultiple: allowMultiple,
    );
    if (result == null) return const [];

    return result.files
        .where((file) => file.path != null)
        .map(
          (file) => MediaAsset(
            id: '${DateTime.now().microsecondsSinceEpoch}_${file.name}',
            path: file.path!,
            type: assetType,
            displayName: file.name,
          ),
        )
        .toList(growable: false);
  }
}
