#pragma once

#include "BaseImage.h"

namespace ofxKinectForWindows2 {
	namespace Source {
		class Depth : public BaseImageSimple<unsigned short, IDepthFrameReader, IDepthFrame> {
		public:
			struct PointCloudOptions {
				enum TextureCoordinates {
					None,
					DepthCamera,
					ColorCamera
				};

				PointCloudOptions();
				PointCloudOptions(bool stitchFaces, bool useColor, TextureCoordinates textureCoordinates);

				bool stitchFaces;
				TextureCoordinates textureCoordinates;
			};

			string getTypeName() const override;
			void init(IKinectSensor *) override;

			ofMesh getMesh(bool stitchFaces, PointCloudOptions::TextureCoordinates textureCoordinates);
			ofMesh getMesh(const PointCloudOptions & pointCloudOptions = PointCloudOptions());

			ICoordinateMapper * getCoordinateMapper();

		protected:
			ICoordinateMapper * coordinateMapper;
		};
	}
}