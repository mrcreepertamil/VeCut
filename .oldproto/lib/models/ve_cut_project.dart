/// Settings for a VeCut editing project.
class VeCutProject {
  const VeCutProject({
    required this.name,
    required this.resolution,
    required this.frameRate,
  });

  final String name;
  final String resolution;
  final int frameRate;
}
