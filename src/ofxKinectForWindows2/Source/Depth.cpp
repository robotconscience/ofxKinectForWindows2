#include "Depth.h"
#include "ofMain.h"

namespace ofxKinectForWindows2 {
	namespace Source {
		//----------
		Depth::PointCloudOptions::PointCloudOptions() {
			this->stitchFaces = true;
			this->textureCoordinates = TextureCoordinates::None;
		}

		//----------
		Depth::PointCloudOptions::PointCloudOptions(bool stitchFaces, bool useColor, TextureCoordinates textureCoordinates) {
			this->stitchFaces = stitchFaces;
			this->textureCoordinates = textureCoordinates;
		}

		//----------
		string Depth::getTypeName() const {
			return "Depth";
		}

		//----------
		void Depth::init(IKinectSensor * sensor) {
			this->reader = NULL;
			try {
				IDepthFrameSource * source = NULL;

				if (FAILED(sensor->get_DepthFrameSource(& source))) {
					throw(Exception("Failed to initialise Depth source"));
				}

				if (FAILED(source->OpenReader(& this->reader))) {
					throw(Exception("Failed to initialise Depth reader"));
				}

				SafeRelease(source);

				if (FAILED(sensor->get_CoordinateMapper(&this->coordinateMapper))) {
					throw(Exception("Failed to acquire coordinate mapper"));
				}
			} catch (std::exception & e) {
				SafeRelease(this->reader);
				throw (e);
			}
		}

		//----------
		ofMesh Depth::getMesh(bool stitchFaces, PointCloudOptions::TextureCoordinates textureCoordinates) {
			const auto frameSize = this->pixels.size();
			const int width = this->getWidth();
			const int height = this->getHeight();

			ofMesh mesh;
			mesh.setMode(stitchFaces ? ofPrimitiveMode::OF_PRIMITIVE_TRIANGLES : ofPrimitiveMode::OF_PRIMITIVE_POINTS);
			mesh.getVertices().resize(frameSize);
			auto vertices = mesh.getVerticesPointer();
			
			this->coordinateMapper->MapDepthFrameToCameraSpace(frameSize, this->pixels.getPixels(), frameSize, (CameraSpacePoint*) mesh.getVerticesPointer());

			if (stitchFaces) {
				for(int i=0; i<width-1; i++) {
					for(int j=0; j<height-1; j++) {
						auto topLeft = width * j + i;
						auto topRight = topLeft + 1;
						auto bottomLeft = topLeft + width;
						auto bottomRight = bottomLeft + 1;
						
						//upper left triangle
						if (vertices[topLeft].z > 0 && vertices[topRight].z > 0 && vertices[bottomLeft].z > 0) {
							const ofIndexType indices[3] = {topLeft, bottomLeft, topRight};
							mesh.addIndices(indices, 3);
						}

						//bottom right triangle
						if (vertices[topRight].z > 0 && vertices[bottomRight].z > 0 && vertices[bottomLeft].z > 0) {
							const ofIndexType indices[3] = {topRight, bottomRight, bottomLeft};
							mesh.addIndices(indices, 3);
						}
					}
				}
			}

			switch(textureCoordinates) {
			case PointCloudOptions::TextureCoordinates::ColorCamera:
				{
					mesh.getTexCoords().resize(frameSize);
					this->coordinateMapper->MapDepthFrameToColorSpace(frameSize, this->pixels.getPixels(), frameSize, (ColorSpacePoint*) mesh.getTexCoordsPointer());
				}
				break;
			case PointCloudOptions::TextureCoordinates::DepthCamera:
				{
					mesh.getTexCoords().resize(frameSize);
					auto texCoords = mesh.getTexCoordsPointer();
					for(int i=0; i<frameSize; i++) {
						texCoords[i] = ofVec2f(i % width, i / width);
					}
				}
				break;
			case PointCloudOptions::TextureCoordinates::None:
			default:
				break;
			}

			return mesh;
		}

		//----------
		ofMesh Depth::getMesh(const PointCloudOptions & pointCloudOptions) {
			return this->getMesh(pointCloudOptions.stitchFaces, pointCloudOptions.textureCoordinates);
		}
		
		//----------
		ICoordinateMapper * Depth::getCoordinateMapper(){
			return coordinateMapper;
		}
	}
}