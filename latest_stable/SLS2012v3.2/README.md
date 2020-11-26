# HOW TO SLS2012v3.2

## Canon Camera configuration

### EOS M50

## Projector configuration

### Resolution
-


##### Pre-requisites:
- 1 USB type 3 DATA cable.

### Scan 

Issues:
- Download request is not working. Bug with the Event Handler?


### [Camera Events](file:///C:/sources/dependencies/EDSDKv131210W/Windows/EDSDK/Header/EDSDKTypes.h)

State Event Code | State Event Label | Notes
-----------|-------------|------------|
 0x00000302 | kEdsStateEvent_JobStatusChanged |
 0x00000305 | kEdsStateEvent_CaptureError |
 0x00000311 | kEdsStateEvent_PowerZoomInfoChanged |

Object Event Code | Object Event Label | Notes
-----------|-------------|------------|
 0x00000201 | kEdsObjectEvent_VolumeInfoChanged | 
 0x00000204 | kEdsObjectEvent_DirItemCreated | 

Property Event Code | Property Event Label | Notes
-----------|-------------|------------|
 0x00000101 | kEdsPropertyEvent_PropertyChanged |
 0x00000102 | kEdsPropertyEvent_PropertyDescChanged |

Property ID | Property ID Label | Param | Notes
-----------|-------------|------------|------------|
 0x00000009 | kEdsPropID_CFn | 0x101 0x102 0x104-0x106 0x108 0x10C 0x10D 0x112 0x114 0x706 0x711-0x714 0x70C 0x813-0x815 0x80E | Camera Setting Properties / Custom Function settings
 0x00000101 | kEdsPropertyID_ | 0x2 0x5 0x6 | Image Properties 
 0x00000114 | kEdsPropertyID_PictureStyle | 0x21-0x23 | Image Properties / Picture style
 0x00000115 | kEdsPropertyID_PictureStyleDesc | 0x81-0x88, 0x21-0x23 | Image Properties / Picture style setting details
 0x00000406 | kEdsPropertyID_Tv | | Capture properties / Shutter speed setting value
 0x0000040A | kEdsPropertyID_AvailableShots | | Capture properties / Number of available shots


### Load Canon batteries
- Use Power cable and Canon charger

### Reconstruct Algorithm (Plant UML Sequence Diagrams)
See `sls-reconstruct.wsd`

### Parameters in `slsConfig.xml`
File is parsed when SLS starts
Parameters | Notes
-----------|-------------|
 Project width | represent the `proj_w` in the code
 Project height | represent the `proj_w` in the code
 Camera width | represent the `cam_w` in the code
 Export GridPly | export faces in the `.ply` file


### Reconstruction phase

```plantumlcode
@startuml
participant SLS
participant Reconstructor
participant Utilities
participant PointCloudImage
participant OpenCV

SLS -> SLS : loadConfiguration("slsConfig.xml")
SLS -> Reconstructor : runReconstruction
activate Reconstructor
    loop n Cameras
        Reconstructor -> Reconstructor : cam2WorldSpace
        Reconstructor -> Reconstructor : loadCamImgs
        Reconstructor -> Reconstructor : computeShadows
        Reconstructor -> Reconstructor : decodePatterns
    end
    loop c1 Cameras
        loop c2 Cameras
            Reconstructor -> Reconstructor : triangulation (c1, c2)
            note right : Console Message "Computing 3D Cloud"
            activate Reconstructor
            loop width Projector
                loop heigth Projector
                    loop p1 Camera Pixel
                        Reconstructor -> Utilities : undistortPoints(p1, c1)
                        Reconstructor -> Utilities : pixelToImageSpace(p1, c1)
                        Reconstructor -> Reconstructor : cam2WorldSpace(p1, c1)                        
                        loop p2 Camera Pixel
                            Reconstructor -> Utilities : undistortPoints(p2, c2)
                            Reconstructor -> Utilities : pixelToImageSpace(p2, c2)
                            Reconstructor -> Reconstructor : cam2WorldSpace(p2, c2)
                            Reconstructor -> Utilities : line_lineIntersection(c1, c2)
                            Utilities --> Reconstructor : interPt
                            Reconstructor -> PointCloudImage : addPoint(interPt)
                            activate PointCloudImage
                                PointCloudImage -> Utilities : matSet3D
                                Utilities -> CV
                            deactivate PointCloudImage
                        end
                    end
                end
            end
            deactivate Reconstructor
        end
    end
deactivate Reconstructor

@enduml
```

