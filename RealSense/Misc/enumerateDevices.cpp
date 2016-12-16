#include <pxccapture.h>

// session is a PXCSession instance
PXCSession::ImplDesc desc1={};
desc1.group=PXCSession::IMPL_GROUP_SENSOR;
desc1.subgroup=PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;

//for (int m=0;;m++) {

    PXCSession::ImplDesc desc2;
    if (session->QueryImpl(&desc1,m,&desc2)<PXC_STATUS_NO_ERROR) break;
    wprintf_s(L"Module[%d]: %s\n", m, desc2.friendlyName);
    PXCCapture *capture=0;
    pxcStatus sts=session->CreateImpl<PXCCapture>(&desc2,&capture);

    if (sts<PXC_STATUS_NO_ERROR) continue;

        // print out all device information
        for (int d=0;;d++) {
            PXCCapture::DeviceInfo dinfo;
            sts=capture->QueryDeviceInfo(d,&dinfo);
            if (sts<PXC_STATUS_NO_ERROR) break;
            wprintf_s(L"    Device[%d]: %s\n",d,dinfo.name);
    }

    capture->Release();

//}
 
 

 
